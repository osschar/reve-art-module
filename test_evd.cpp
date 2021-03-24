#include "MockEventDisplayManager.h"
#include <ROOT/RWebWindow.hxx>
#include "ROOT/RWebWindowsManager.hxx"

#include "ROOT/REveElement.hxx"
#include "ROOT/REveManager.hxx"
#include "ROOT/REveScene.hxx"
#include "ROOT/REvePointSet.hxx"

#include "TApplication.h"
#include "TROOT.h"
#include "TSystem.h"
#include "TRandom.h"

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>

namespace REX = ROOT::Experimental;

namespace {

  class MockModule {
  public:
    void signalAppStart();
    void beginJob();
    void analyze(unsigned event_id);
    void endJob();

  private:
    void setup_eve();
    void run_application();
    void process_single_event();

    // Application control
    TApplication application_{"none", nullptr, nullptr};
    std::thread appThread_{};

    // Control between the main thread and event-display thread
    std::mutex m_{};
    std::condition_variable cv_{};

    unsigned displayedEventID_{test::invalid_event};
    REX::REveManager* eveMng_{nullptr};
    test::MockEventDisplayManager* eventMgr_{nullptr};
  };

  class XThreadTimer : public TTimer {
    std::function<void()> foo_;
  public:
    XThreadTimer(std::function<void()> f) : foo_(f)
    {
      SetTime(0);
      R__LOCKGUARD2(gSystemMutex);
      gSystem->AddTimer(this);
    }
    Bool_t Notify() override
    {
      foo_();
      gSystem->RemoveTimer(this);
      return kTRUE;
    }
  };

  void
  MockModule::signalAppStart()
  {
    std::unique_lock lock{m_};
    cv_.notify_all();
  }

  void
  MockModule::beginJob()
  {
    {
      std::unique_lock lock{m_};

      appThread_ = std::thread{[this] { run_application(); }};

      // Wait for app init to finish ... this will process pending timer events.

      XThreadTimer sut([this]{ signalAppStart(); });
      printf("MockModule::beginJob -- starting wait on app start\n");
      cv_.wait(lock);
      printf("MockModule::beginJob -- app start signal received, starting eve init\n");

      XThreadTimer suet([this]{ setup_eve(); });
      printf("MockModule::beginJob -- starting wait on eve setup\n");
      cv_.wait(lock);
      printf("MockModule::beginJob -- eve setup apparently complete\n");
    }
  }

  void
  MockModule::analyze(unsigned int const event_id)
  {
    // Update state relevant for displaying new event.
    displayedEventID_ = event_id;

    // Hand off control to display thread
    std::unique_lock lock{m_};

    if (displayedEventID_ != test::invalid_event)
    {
      printf("MockModule::analyze() Event processing started entered\n");

      XThreadTimer proc_timer([this]{ process_single_event(); });
      printf("MockModule::event_display_loop -- transferring to TApplication thread\n");
      cv_.wait(lock);
      printf("MockModule::event_display_loop -- TApplication thread returning control\n");
    }
  }

  void
  MockModule::endJob()
  {
    application_.Terminate(0);

    if (appThread_.joinable()) {
      appThread_.join();
    }
  }

  // Functions invoked by the timers created by this module

  void
  MockModule::run_application()
  {
    // Without this the startup timer might not get invoked.
    gSystem->ProcessEvents();
    application_.Run(true);
  }

  void
  MockModule::setup_eve()
  {
    REX::RWebWindowsManager::AssignMainThrd();
    eveMng_ = REX::REveManager::Create();

    eventMgr_ = new test::MockEventDisplayManager{eveMng_, cv_, m_};
    auto world = eveMng_->GetWorld();
    assert(world);
    world->AddElement(eventMgr_);
    world->AddCommand("QuitRoot",  "sap-icon://log",  eventMgr_, "QuitRoot()");
    world->AddCommand("NextEvent", "sap-icon://step", eventMgr_, "NextEvent()");

    eveMng_->Show();

    std::unique_lock lock{m_};
    cv_.notify_all();
  }

  void
  MockModule::process_single_event()
  {    
    eveMng_->DisableRedraw();
    eveMng_->GetWorld()->BeginAcceptingChanges();
    eveMng_->GetScenes()->AcceptChanges(true);
    
    auto scene = eveMng_->GetEventScene();
  //  scene->DestroyElements();
    // for (auto& ie : scene->RefChildren()) {
    //   // Handle scene elements ...
    // }
    TRandom &r = *gRandom;
    int npoints = 100; float s  =2;
    auto ps = new REX::REvePointSet("fu", "", npoints);

    for (Int_t i=0; i<npoints; ++i)
       ps->SetNextPoint(r.Uniform(-s,s), r.Uniform(-s,s), r.Uniform(-s,s));

    ps->SetMarkerColor(r.Integer(700));
    ps->SetMarkerSize(3+r.Uniform(1, 7));
    ps->SetMarkerStyle(4);
    scene->AddElement(ps);

    eveMng_->GetScenes()->AcceptChanges(false);
    eveMng_->GetWorld()->EndAcceptingChanges();
    eveMng_->EnableRedraw();
  }
  
} // namespace

int
main()
{
  ROOT::EnableThreadSafety();
  //gDebug = 1;

  MockModule module;
  module.beginJob();

  constexpr unsigned n_events = 10;
  try {
    // Event loop runs on main thread
    for (unsigned i = 0; i != n_events; ++i) {
      module.analyze(i);
    }
  }
  catch (...) {
  }

  module.endJob();
}
