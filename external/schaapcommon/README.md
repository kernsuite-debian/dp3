# schaapcommon
This repository contains utilities that are shared among different packages in the schaap-stack, including DP3/WSClean and EveryBeam.

## Requirements
When compiling `schaapcommon` as a stand-alone (static) library, the `aocommon` headers need to be available. `aocommon` can be cloned from https://gitlab.com/aroffringa/aocommon. To include the headers in the (cmake) build process, use the `AOCOMMON_INCLUDE_DIR` variable.

A `cmake` command typically reads

```
cmake -DAOCOMMON_INCLUDE_DIR=[PATH_TO_AOCOMMON/aocommon/include] -DCMAKE_INSTALL_PREFIX=[INSTALL_PATH] [PATH_TO_SCHAAPCOMMON]
```
