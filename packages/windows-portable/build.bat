@if "%~1"=="" goto usage
@if "%~2"=="" goto usage


@set BUILD_DIRECTORY="%~1"
@set ARCHITECTURE="%~2"


@cmake -B "%BUILD_DIRECTORY%" -DPORTABLE=True -A "%ARCHITECTURE%"
@if %ERRORLEVEL% NEQ 0 goto error
@cmake --build "%BUILD_DIRECTORY%" --config Release --target package
@if %ERRORLEVEL% NEQ 0 goto error


@goto end


:usage
@echo Usage: build.bat (build directory) (architecture to build)
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