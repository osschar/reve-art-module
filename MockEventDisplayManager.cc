#include "MockEventDisplayManager.h"

#include "ROOT/REveManager.hxx"
#include "ROOT/REveScene.hxx"

#include <stdexcept>
#include <iostream>

namespace test {

  MockEventDisplayManager::MockEventDisplayManager(
    ROOT::Experimental::REveManager* eveMgr,
    std::condition_variable& cv,
    std::mutex& m)
    : REveElement{"EventManager"}, eveMng_{eveMgr}, cv_{&cv}, m_{&m}
  {}

  void
  MockEventDisplayManager::NextEvent()
  {
    std::unique_lock lock{*m_};
    cv_->notify_all();
  }

  void
  MockEventDisplayManager::QuitRoot()
  {
    // This will wreak havoc if not thrown from the main thread.
    doneProcessingEvents_ = true;
    std::unique_lock<std::mutex> lock{*m_};
    cv_->notify_all();
    // throw std::runtime_error("EventDisplay");
  }

}
