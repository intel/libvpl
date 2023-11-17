This directory contains oneAPI Video Processing Library (oneVPL) example code.

Please note: this code is provided to illustrate API concepts only.  It is not intended for use in production code or for performance benchmarks.

# api1x_core:
These examples demonstrate use of the legacy 1.x core API subset which remains portable across hardware enabled by both
Intel® Media SDK and oneVPL GPU runtimes.  (The core API subset is Media SDK 1.x API - features removed in oneVPL.)
This style of coding can use oneVPL implementation capability queries but applications must manage surface allocation.  
Use this approach for maximum portability when legacy hardware support is required.
For more information see our [Transition Guide](https://www.intel.com/content/www/us/en/develop/documentation/upgrading-from-msdk-to-onevpl/top.html)

# api2x:
This group of examples demonstrates use of 2.x API capabilities including internal allocation.  This means that applications
do not need to manage surface allocation lifecycles.
Use of these examples requires a runtime implementation compatible with the 
VPL 2.x API, such as the [oneVPL GPU Runtime](https://github.com/oneapi-src/oneVPL-intel-gpu).  
Please note: the [Intel® Media SDK runtime](https://github.com/Intel-Media-SDK/MediaSDK) only provides API 1.35, and is not
compatible with these examples.

# interop:
The examples in this group showcase pipelines combining oneVPL media capabilities with other APIs such as the OpenVINO™ Toolkit Interface.
