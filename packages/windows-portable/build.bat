@if "%~1"=="" goto usage
@if "%~2"=="" goto usage
@if "%~3"=="" goto usage


@set SOURCE_DIRECTORY="%~1"
@set BUILD_DIRECTORY="%~2"
@set ARCHITECTURE="%~3"


@cmake -S "%SOURCE_DIRECTORY%" -B "%BUILD_DIRECTORY%" -DPORTABLE=1 -DFETCH_DEPENDENCIES=1 -DVCPKG_MANIFEST_MODE=0 -A "%ARCHITECTURE%"
@if %ERRORLEVEL% NEQ 0 goto error
@cmake --build "%BUILD_DIRECTORY%" --config Release --target package
@if %ERRORLEVEL% NEQ 0 goto error


@goto end


:usage
@echo Usage: build.bat (source directory) (build directory) (architecture to build)
@echo Supported architectures to build:
@echo Win32
@echo x64
@echo ARM
@echo ARM64
@goto end


:error
@echo Error encountered
@goto end


:end