function(add_sources)

	message(STATUS "sources: ${ARGV}")
	target_sources(app PRIVATE ${ARGN})

endfunction()

function(add_sources_if condition)

	if(${${condition}})
		add_sources(${ARGN})
	endif()

endfunction()


function(add_glob_sources)

	file(GLOB_RECURSE FILES ${ARGN})
	add_sources(${FILES})

endfunction()

function(add_glob_sources_if condition)

	if(${${condition}})
		add_glob_sources(${ARGN})
	endif()

endfunction()
