add_executable(Launcher)


# Set project properties
set_property(TARGET Launcher PROPERTY VS_DOTNET_TARGET_FRAMEWORK_VERSION "v4.8")
set_property(TARGET Launcher PROPERTY WIN32_EXECUTABLE FALSE)
set_property(TARGET Launcher PROPERTY VS_CONFIGURATION_TYPE Console)
set_property(TARGET Launcher PROPERTY VS_DOTNET_REFERENCES_COPY_LOCAL ON)

# Add System to references
set_property(TARGET Launcher PROPERTY VS_DOTNET_REFERENCES "System")

# Add sources
target_sources(Launcher PRIVATE
	"App.cs"
)