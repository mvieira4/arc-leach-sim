file(REMOVE_RECURSE
  "../../build/lib/libns3.37-leach-default.pdb"
  "../../build/lib/libns3.37-leach-default.so"
)

# Per-language clean rules from dependency scanning.
foreach(lang CXX)
  include(CMakeFiles/libleach.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
