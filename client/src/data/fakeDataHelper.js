/**
 * Static methods for creating fake graphGLWindow data.
 */
const FakeDataHelper = {
  /**
   * Creates a fake set of node [x,y] positions for testing.
   * @return {Array}
   */
  createFakeNodePositions: function() {
    let fakeNodesPositions = [];
    let radius = 0.5;
    let pointCount = 16;
    let angle = Math.PI * 2 / pointCount;
    for (let i = 0; i < pointCount; i++) {
      let pX = radius * Math.cos(angle * i);
      let pY = radius * Math.sin(angle * i);
      fakeNodesPositions.push([pX, pY]);
    }
    return fakeNodesPositions;
  },


  /**
   * Creates a fake set of edge indices for testing.
   * @param {Array} nodePositions
   * @return {Array}
   */
  createFakeEdges: function(nodePositions) {
    let fakeEdgesIndices = [];

    for (let i = 0; i < nodePositions.length; i++) {
      let min = 0;
      let max = nodePositions.length - 1;
      min = Math.ceil(min);
      max = Math.floor(max);
      let randomNodeIndex = Math.floor(Math.random() * (max - min)) + min;
      fakeEdgesIndices.push([i, randomNodeIndex]);
    }

    for (let j = 0; j < 16; j++) {
      let min = 0;
      let max = nodePositions.length - 1;
      min = Math.ceil(min);
      max = Math.floor(max);
      let randomFirst = Math.floor(Math.random() * (max - min)) + min;
      let randomSecond = Math.floor(Math.random() * (max - min)) + min;
      while (randomSecond == randomFirst) {
        randomSecond = Math.floor(Math.random() * (max - min)) + min;
      }
      fakeEdgesIndices.push([randomFirst, randomSecond]);
    }
    return fakeEdgesIndices;
  },

  /**
   * Creates fakeNodeColors for proof of concept
   * @param {object} nodes
   * @return {Array}
   */
  createFakeNodeColors: function(nodes) {
    let fakeNodeColors = [];
    for (let i = 0; i < nodes.length; i++) {
      let r = 1.0 - (1.0 / nodes.length * i);
      let g = 0.8;
      let b = 0.0 + (1.0 / nodes.length * i);
      fakeNodeColors.push(r, g, b);
    }
    return fakeNodeColors;
  },
};

export default FakeDataHelper;
