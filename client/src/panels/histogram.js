import * as d3 from 'd3';
import React from 'react';

/**
 * A component to render histograms.
 */
class Histogram extends React.Component {
  /**
   * Histogram constructor.
   * @param {object} props
   */
  constructor(props) {
    super(props);

    this.createBarChart = this.createBarChart.bind(this);
  }

  /**
   * Callback invoked immediately after the component is mounted.
   */
  componentDidMount() {
    this.createBarChart();
  }

  /**
   * Callback invoked immediately after the component is updated.
   */
  componentDidUpdate() {
    this.createBarChart();
  }

  /**
   * Create the chart using d3 and svg.
   */
  createBarChart() {
    const svg = this.refs.svgRoot;
    const dataMax = Math.max(...this.props.data);
    this.yScale = d3.scaleLinear()
      .domain([0, dataMax])
      .range([0, this.props.size[1]]);

    d3.select(svg)
      .selectAll('rect')
      .data(this.props.data)
      .enter()
      .append('rect');

    this.xScale = d3.scaleLinear()
      .domain([0, this.props.data.length])
      .range([0, this.props.size[0]]);
    let xAxis = d3.axisBottom(this.xScale).ticks(0);

    d3.select(svg)
      .selectAll('rect')
      .data(this.props.data)
      .exit()
      .remove();

    d3.select(svg)
      .append('g')
      .attr('transform', 'translate(0,100)')
      .call(xAxis);

    let barWidth = this.props.size[0] / this.props.data.length;

    d3.select(svg)
      .selectAll('rect')
      .data(this.props.data)
      .style('fill', '#3f51b5')
      .style('stroke-width', '1')
      .style('stroke', 'rgb(255,255,255)')
      .attr('x', (d, i) => i * barWidth)
      .attr('y', (d) => this.props.size[1] - this.yScale(d))
      .attr('height', (d) => this.yScale(d))
      .attr('width', barWidth);

    // Add brushing capability - if enabled
    if (this.props.brushEnabled) {
      d3.select(svg)
        .attr('class', 'brush')
        .call(d3.brush().on('brush', () => this.props.onBrush(this.xScale.invert(d3.event.selection[0][0]),
          this.xScale.invert(d3.event.selection[1][0]))));
    }
  }

  /**
   * Renders the component to HTML.
   * @return {HTML}
   */
  render() {
    return <svg ref='svgRoot' width={this.props.size[0]} height={110}/>;
  }
}

export default Histogram;
