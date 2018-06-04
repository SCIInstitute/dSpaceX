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
  constructor(centerX, centerY, width, height) {
    this.X = centerX; // center of quad
    this.Y = centerY;
    this.width = width;
    this.height = height;

    this.updateVertices();
  }

  /**
   * increase radius
   * will increase the width and height by scaler value
   * will update the vertice positions
   * @param {number} scaler
   */
  increaseRadius(scaler) {
    this.width *= scaler;
    this.height *= scaler;

    this.updateVertices();
  }

  /**
   * decrease radius
   * will decrease the width and height by scaler value
   * will update the vertice positions
   * @param {number} scaler
   */
  decreaseRadius(scaler) {
    this.width = Math.max(0.002, this.width / scaler);
    this.height = Math.max(0.002, this.height / scaler);

    this.updateVertices();
  }

  /**
   * update vertice values
   */
  updateVertices() {
    let minX = -(this.width / 2.0) + this.X;
    let maxX = minX + this.width;
    let minY = -(this.height / 2.0) + this.Y;
    let maxY = minY + this.height;

    // using vec3 as (x, y, UV)
    this.vertices = [
      minX, maxY, 0,
      maxX, maxY, 1,
      minX, minY, 2,
      minX, minY, 2,
      maxX, maxY, 1,
      maxX, minY, 3];
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
   * @param {number} lineWidth
   */
  constructor(x1, y1, x2, y2, lineWidth = 0.0025) {
    this.x1 = x1;
    this.y1 = y1;
    this.x2 = x2;
    this.y2 = y2;

    // Setup properties for drawing as quad
    this.width = lineWidth;
    let vectorX = x2 - x1;
    let vectorY = y2 - y1;
    let lineVector = [vectorX, vectorY];
    this.offsetVector = [lineVector[1], -lineVector[0]];

    // Normalize the offset vector
    let mag = Math.abs(Math.sqrt((this.offsetVector[0] * this.offsetVector[0])
      + (this.offsetVector[1] * this.offsetVector[1])));
    this.offsetVector = [this.offsetVector[0]/mag, this.offsetVector[1]/mag];

    this.updateVertices();
  }

  /**
   * increase thickness
   * will increase the width by scaler value
   * will update the vertice positions
   * @param {number} scaler
   */
  increaseThickness(scaler) {
    this.width *= scaler;
    this.updateVertices();
  }

  /**
   * decrease thickness
   * will decrease the width by scaler value
   * will update the vertice positions
   * @param {number} scaler
   */
  decreaseThickness(scaler) {
    this.width = Math.max(0.0001, this.width / scaler);
    this.updateVertices();
  }

  /**
   * update vertice values
   */
  updateVertices() {
    let p1 = [this.x1 + this.offsetVector[0] * this.width,
      this.y1 + this.offsetVector[1] * this.width];
    let p2 = [this.x1 - this.offsetVector[0] * this.width,
      this.y1 - this.offsetVector[1] * this.width];
    let p3 = [this.x2 + this.offsetVector[0] * this.width,
      this.y2 + this.offsetVector[1] * this.width];
    let p4 = [this.x2 - this.offsetVector[0] * this.width,
      this.y2 - this.offsetVector[1] * this.width];

    // using vec3 as (x, y, UV)
    this.vertices = [
      p1[0], p1[1], 0,
      p2[0], p2[1], 1,
      p3[0], p3[1], 2,
      p3[0], p3[1], 2,
      p2[0], p2[1], 1,
      p4[0], p4[1], 3];
  }
}
