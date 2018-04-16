/**
 * Creates a basic quad with 4 vertices and 6 indices.
 */
export class Quad {
  /**
   * Quad constructor.
   * @param {number} centerX
   * @param {number} centerY
   * @param {number} width
   * @param {number} height
   * @param {number} firstIndex
   */
  constructor(centerX, centerY, width, height, firstIndex) {
    this.X = centerX; // center of quad
    this.Y = centerY;
    this.width = width;
    this.height = height;

    let minX = -(width / 2.0) + centerX;
    let maxX = minX + width;
    let minY = -(height / 2.0) + centerY;
    let maxY = minY + height;

    this.vertices = [
      minX, maxY,
      maxX, maxY,
      minX, minY,
      maxX, minY];

    let n = firstIndex;
    this.indices = [
      n, n + 1, n + 2,
      n + 2, n + 1, n + 3];
  }
}

/**
 * Creates a basic edge from 2 points
 */
export class Edge {
  /**
   * Edge constructor
   * @param {number} x1
   * @param {number} y1
   * @param {number} x2
   * @param {number} y2
   */
  constructor(x1, y1, x2, y2) {
    this.x1 = x1;
    this.y1 = y1;
    this.x2 = x2;
    this.y2 = y2;
  }
}

