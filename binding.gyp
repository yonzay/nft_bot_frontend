{
  "targets": [
    {
      "target_name": "Module",
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "conditions": [
        ["OS=='win'", {
          "defines": [
            "_HAS_EXCEPTIONS=1"
          ],
          "msvs_settings": {
            "VCCLCompilerTool": {
              "ExceptionHandling": 1
            },
          },
        }]
      ],
      "sources": [
        "module/src/Module.cpp",
        "module/src/Action.cpp",
        "module/src/Analytics.cpp",
        "module/src/Auth.cpp",
        "module/src/Chronicle.cpp",
        "module/src/Cryptography.cpp",
        "module/src/Http_Request.cpp",
        "module/src/Instance_Summary.cpp",
        "module/src/Session.cpp",
        "module/src/Settings.cpp",
        "module/src/Utility.cpp",
        "module/src/Environment.cpp",
        "module/src/Contract_Task.cpp",
        "module/src/Sniper_Task.cpp",
        "module/src/Wallet_Group.cpp"
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "module/include/"
      ]
    }
  ]
}