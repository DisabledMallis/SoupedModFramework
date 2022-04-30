add_library(ModFS SHARED)

target_include_directories(ModFS PUBLIC "include")

target_sources(ModFS PRIVATE
	"modfs.cpp"
)