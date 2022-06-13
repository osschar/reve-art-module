#ifndef MockEventDisplayManager_h
#define MockEventDisplayManager_h

#include "ROOT/REveElement.hxx"

#include <condition_variable>
#include <limits>
#include <mutex>

namespace ROOT::Experimental {
  class REveManager;
}

namespace test {

  constexpr auto invalid_event = std::numeric_limits<unsigned>::max();

  class MockEventDisplayManager : public ROOT::Experimental::REveElement {
  public:
    MockEventDisplayManager() = default; // ROOT needs a dictionary

    explicit MockEventDisplayManager(ROOT::Experimental::REveManager* eveMgr,
                                     std::condition_variable& cv,
                                     std::mutex& m);

    void NextEvent();
    void QuitRoot();

  private:
    ROOT::Experimental::REveManager* eveMng_{nullptr};
    std::condition_variable* cv_{nullptr};
    std::mutex* m_{nullptr};
    bool doneProcessingEvents_{false};
  };

  class Gui : public ROOT::Experimental::REveElement
  {
  public:
    int fCount{0};
    int fTotal{10};
    int fLumi{1111};

    using REveElement::WriteCoreJson;
    int WriteCoreJson(nlohmann::json &j, int rnr_offset) override;
  };
}

#endif /* MockEventDisplayManager_h */
