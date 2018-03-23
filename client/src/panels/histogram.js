import React from 'react';
import * as d3 from 'd3';


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
    const yScale = d3.scaleLinear()
      .domain([0, dataMax])
      .range([0, this.props.size[1]]);

    d3.select(svg)
      .selectAll('rect')
      .data(this.props.data)
      .enter()
      .append('rect');

    let xRange = d3.scaleLinear()
      .domain([0, this.props.data.length])
      .range([0, this.props.size[0]]);
    let xAxis = d3.axisBottom(xRange).ticks(0);

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
      .attr('y', (d) => this.props.size[1] - yScale(d))
      .attr('height', (d) => yScale(d))
      .attr('width', barWidth);
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
