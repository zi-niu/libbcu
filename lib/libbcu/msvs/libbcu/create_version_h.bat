echo //generated by file create_version_h.bat > ../../include/libbcu_version.h

IF "%APPVEYOR_BUILD_VERSION%"=="" (
	echo build not in appveyor
) ELSE (
	git tag %APPVEYOR_BUILD_VERSION%
)


FOR /F "tokens=*" %%a in ('call git describe --tags --long') do (
		echo #define LIBBCU_GIT_VERSION "lib%%a" > ../../include/libbcu_version.h
)