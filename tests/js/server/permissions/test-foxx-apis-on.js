/*jshint globalstrict:false, strict:false */
/* global getOptions, assertTrue, assertFalse, assertEqual, arango */

////////////////////////////////////////////////////////////////////////////////
/// @brief test for security-related server options
///
/// @file
///
/// DISCLAIMER
///
/// Copyright 2010-2012 triagens GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB Inc, Cologne, Germany
///
/// @author Wilfried Goesgens
/// @author Copyright 2019, ArangoDB Inc, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

if (getOptions === true) {
  let users = require("@arangodb/users");
  
  users.save("test_rw", "testi");
  users.grantDatabase("test_rw", "_system", "rw");
  
  users.save("test_ro", "testi");
  users.grantDatabase("test_ro", "_system", "ro");
  
  return {
    'foxx.api': 'true',
  };
}
var jsunity = require('jsunity');

function testSuite() {
  let endpoint = arango.getEndpoint();
  let db = require("@arangodb").db;
  const errors = require('@arangodb').errors;

  return {
    setUp: function() {},
    tearDown: function() {},

    testCanAccessFoxxApiRw : function() {
      arango.reconnect(endpoint, db._name(), "test_rw", "testi");

      let routes = [
        "setup", "teardown", "install", "uninstall",
        "replace", "upgrade", "configure", "configuration",
        "set-dependencies", "dependencies", "development",
        "tests", "script"
      ];

      routes.forEach(function(route) {
        // foxx API is available. but as we are now posting some random
        // stuff, we cannot expect the requests to work
        let result = arango.POST("/_admin/foxx/" + route, {});
        assertTrue(result.error);
        assertNotEqual(403, result.code);
        assertNotEqual(403, result.errorNum);
        assertNotEqual(errors.ERROR_FORBIDDEN.code, result.errorNum);
      });
    },
    
    testCanAccessFoxxApiRo : function() {
      arango.reconnect(endpoint, db._name(), "test_ro", "testi");

      let routes = [
        "setup", "teardown", "install", "uninstall",
        "replace", "upgrade", "configure", "configuration",
        "set-dependencies", "dependencies", "development",
        "tests", "script"
      ];

      routes.forEach(function(route) {
        // foxx API is available. but as we are now posting some random
        // stuff, we cannot expect the requests to work
        let result = arango.POST("/_admin/foxx/" + route, {});
        assertTrue(result.error);
        assertNotEqual(403, result.code);
        assertNotEqual(403, result.errorNum);
        assertNotEqual(errors.ERROR_FORBIDDEN.code, result.errorNum);
      });
    },

  };
}
jsunity.run(testSuite);
return jsunity.done();
