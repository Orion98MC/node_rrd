{
  "targets": [
    {
      "target_name": "rrd_bindings",
      "sources": [ 
          "src/node_rrd_create.cc"
        , "src/node_rrd_update.cc"
        , "src/node_rrd_fetch.cc"
      	, "src/node_rrd_last.cc"
      	, "src/node_rrd_info.cc"
      	, "src/node_rrd.cc"
      ],
      'libraries': [ "-lrrd" ]
    }
  ]
}