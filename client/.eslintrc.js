module.exports = {
  "parser": "babel-eslint",
  "extends": [
    "plugin:css-modules/recommended",
    "google",
  ],
  "rules": {
    "indent": ["error", 2, { "SwitchCase": 1 }],
    "switch-colon-spacing": 0,
    "object-curly-spacing" : [
      "error", "always", {
        "arraysInObjects": false
      },
    ],
    "max-len":["error",
      {
        "code":120
      }
    ],
    "key-spacing": [
      "error", {
        "singleLine": {
          "beforeColon": false,
          "afterColon": false
        },
        "multiLine": {
          "beforeColon": false,
          "afterColon": true
        }
      }
    ],
    "react/jsx-uses-react": "error",
    "react/jsx-uses-vars": "error",
    "react/jsx-indent": [0 /* off */, 2],
    "brace-style": [ "error", "1tbs", { "allowSingleLine": true }],
    "block-spacing": [ "error", "always"],
    "sort-imports": ["error", {
        "ignoreCase": false,
        "ignoreMemberSort": false,
        "memberSyntaxSortOrder": ["none", "all", "multiple", "single"]
    }]
  },
  "plugins": [
    "react",
    "css-modules"
  ]
};
