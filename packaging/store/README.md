# Store package profile

Run `Test-StoreRuntime.ps1` for a read-only deployed-runtime dependency preflight. `New-StorePackage.ps1` requires the owner's real Partner Center identity and packages that deployment using a MakeAppx file mapping; it does not stage a second runtime copy. `New-StoreAssets.ps1` converts the existing source artwork into the three required logo sizes without changing the source image.

See [Store preparation and commands](../../docs/store-preparation.md) for build switches, required identity values, the 1.5 GB repository limit, signing/installed tests, and remaining certification/submission work. Packaging does not sign, install, certify or publish an app.
