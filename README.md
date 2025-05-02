# reve-art-module

Minimal reproducer that mimics (an attempted) usage of REve in art framework.

ROOT event-loop (TApplication::Run()) is run in a separate (not main) thread.

Functions calling REve functions are executed in TApplication's thread via TTimer, class XThreadTimer.



# build & run
* set ROOT env
* make
* run:
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:. ./test 