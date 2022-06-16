add_executable(Launcher)

set_property(TARGET Launcher PROPERTY VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.8")

target_sources(Launcher PRIVATE
    "installer.cs"
	"main.cs"
    "patcher.cs"
)

add_custom_command(TARGET Launcher
    PRE_BUILD
    COMMAND dotnet restore ${CMAKE_BINARY_DIR}/launcher/Launcher.csproj -r win-x64
)

#set_property(TARGET Launcher PROPERTY VS_PACKAGE_REFERENCES
#    "Microsoft.Web.WebView2_*"
#)

set_property(TARGET Launcher PROPERTY VS_DOTNET_REFERENCES
    "Microsoft.CSharp"
    "System"
    "System.Core"
    "System.Data"
    "System.Drawing"
    "System.Windows.Forms"
)