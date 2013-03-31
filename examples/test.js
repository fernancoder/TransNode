var TransNode = require('../build/Release/TransNode');

TransNode.open(".TransNode", function(res) {
  console.log("TransNode started", res);
});
