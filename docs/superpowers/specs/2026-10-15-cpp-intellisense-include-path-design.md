# C/C++ IntelliSense Include-Path Design

## Goal

Resolve VS Code IntelliSense errors for headers used by the IS25LP128 driver without changing driver source or relying on machine-specific firmware paths.

## Context

The affected driver includes headers such as `Type.h`, `Disp.h`, and `hs0_mmc.h`. These headers are available under the repository's `common/h` directory. The repository currently has no `.vscode/c_cpp_properties.json`, so the C/C++ extension does not know to search that directory.

## Design

Add a repository-level `.vscode/c_cpp_properties.json` that includes `${workspaceFolder}/common/h` and repository device directories in IntelliSense include and browse paths. Keep the configuration portable: do not point it at the machine-specific `D:\\ALL-1000\\ALL1000Code` firmware checkout. Do not change C source, build scripts, compiler selection, or compiler flags.

## Alternatives

- Add only `common/h` to a local user-level setting: minimal, but not shared with other repository users.
- Add the external firmware include directories used by `iBuild.bat`: mirrors the local build environment, but is machine-specific and unnecessary for the headers reported here.

The repository-level configuration is preferred because it resolves the reported project headers consistently and remains portable.

## Validation

- Confirm the JSON configuration is valid and its include paths refer to existing repository directories.
- Confirm every header named in the reported IntelliSense diagnostics exists under `common/h`.
- No firmware build or source-level test is required because only editor configuration changes.
