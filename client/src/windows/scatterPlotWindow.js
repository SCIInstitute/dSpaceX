import * as d3 from 'd3';
import PropTypes from 'prop-types';
import React from 'react';
import { withDSXContext } from '../dsxContext.js';
import { withStyles } from '@material-ui/core/styles';

const styles = (theme) => ({
  root: {
    overflowX: 'auto',
  },
});

const svgContainer = {
  'display': 'inline-block',
  'position': 'relative',
  'width': '100%',
  'verticalAlign': 'top',
  'overflow': 'hidden',
};

const svgContent = {
  'display': 'inline-block',
  'position': 'absolute',
  'top': 0,
  'left': 0,
  'fontSize': '1em',

};

/**
 * This class provides the functionality for the d3 scatter plot.
 * Currently only works for Ellipse example data set.
 */
class ScatterPlotWindow extends React.Component {
  constructor(props) {
    super(props);

    this.client = this.props.dsxContext.client;

    this.svgWidth = 1000;
    this.svgHeight = 600;

    this.state = {
      parameters: [],
      qois: [],
    };

    this.getParameters = this.getParameters.bind(this);
    this.getQois = this.getQois.bind(this);
    this.drawChart = this.drawChart.bind(this);
    this.areAxesSet = this.areAxesSet.bind(this);
    this.getData = this.getData.bind(this);
    this.areMarkersSet = this.areMarkersSet.bind(this);
  }

  /**
   * This gets the parameters for the dataset
   * @return {Promise<Array>}
   */
  async getParameters() {
    const { datasetId, parameterNames } = this.props.dataset;
    let parameters = [];
    parameterNames.forEach(async (parameterName) => {
      let parameter =
        await this.client.fetchParameter(datasetId, parameterName);
      parameters.push(parameter);
    });
    return parameters;
  }

  /**
   * This get the QOIs for the dataset
   * @return {Array}
   */
  async getQois() {
    const { datasetId, qoiNames } = this.props.dataset;
    let qois = [];
    qoiNames.forEach(async (qoiName) => {
      let qoi =
        await this.client.fetchQoi(datasetId, qoiName);
      qois.push(qoi);
    });
    return qois;
  }

  /**
   * Verify x and y axes values are set
   * @return {boolean}
   */
  areAxesSet() {
    const { xAttributeGroup, xAttribute, yAttributeGroup, yAttribute } = this.props.config;
    if (xAttributeGroup === undefined) {
      return false;
    }
    if (xAttribute === undefined) {
      return false;
    }
    if (yAttributeGroup === undefined) {
      return false;
    }
    return yAttribute !== undefined;

  }

  /**
   * Verify marker values are set
   * @returns {boolean}
   */
  areMarkersSet() {
    const { markerAttributeGroup, markerAttribute } = this.props.config;
    if (markerAttributeGroup === undefined) {
      return false;
    }
    return markerAttribute !== undefined;

  }

  componentWillMount() {
    this.getParameters().then((data) => {
      this.setState({ parameters:data } );
    });
    this.getQois().then((data) => {
      this.setState({ qois:data } );
    });
    if (this.areAxesSet()) {
      this.drawChart();
    }
  }

  componentDidUpdate(prevProps) {
    if (this.areAxesSet()) {
      this.drawChart();
    }
  }

  getData(attributeGroup, attribute) {
    if (attributeGroup === 'parameters') {
      return this.state.parameters.filter((p) => p.parameterName === attribute)[0].parameter;
    } else {
      return this.state.qois.filter((q) => q.qoiName === attribute)[0].qoi;
    }
  }

  drawChart() {
    // Get this node
    const node = this.node;
    // d3.select(node).selectAll('*').remove(); TODO remove this line if possible; update correctly

    // Get the data
    const { xAttributeGroup, xAttribute, yAttributeGroup, yAttribute,
      markerAttributeGroup, markerAttribute } = this.props.config;

    let markerValues = [];
    if (this.areMarkersSet()) {
      markerValues = this.getData(markerAttributeGroup, markerAttribute);
    }

    // Create margins
    let margin = { top:50, right:50, bottom:50, left:50 };
    let chartWidth = this.svgWidth - margin.left - margin.right;
    let chartHeight = this.svgHeight - margin.top - margin.bottom;


    // Create scales
    const xValues = this.getData(xAttributeGroup, xAttribute);
    let xScale = d3.scaleLinear()
      .range([0, chartWidth])
      .domain([0, d3.max(xValues)])
      .nice();

    const yValues = this.getData(yAttributeGroup, yAttribute);
    let yScale = d3.scaleLinear()
      .range([chartHeight, 0])
      .domain([0, d3.max(yValues)])
      .nice();

    // Add axes
    let xAxis = d3.axisBottom(xScale);
    d3.select(node)
      .append('g')
      .attr('transform',
        'translate(' + margin.left + ',' + (chartHeight + margin.top) + ')')
      .call(xAxis);

    let yAxis = d3.axisLeft(yScale);
    d3.select(node)
      .append('g')
      .attr('transform',
        'translate(' + margin.left + ',' + margin.top + ')')
      .call(yAxis);
    //
    // // Add Labels
    // d3.select(node).append('text')
    //   .attr('x', (margin.top + padding.small))
    //   .attr('y', (-1 * (margin.left + padding.x_small)))
    //   .attr('transform', 'rotate(90)')
    //   .text(columnNames[0]);
    //
    // d3.select(node).append('text')
    //   .attr('x', (width + padding.small))
    //   .attr('y', (height + margin.bottom - padding.x_small))
    //   .attr('text-anchor', 'end')
    //   .text(columnNames[1]);
    //
    // // Add data
    // let circles = d3.select(node).append('g')
    //   .selectAll('circle')
    //   .data(data);
    // let circlesEntering = circles.enter().append('circle');
    // circles.exit().remove();
    // circles = circles.merge(circlesEntering);
    //
    // circles
    //   .attr('cx', (d) => xScale(d[columnNames[0]]))
    //   .attr('cy', (d) => yScale(d[columnNames[1]]))
    //   .attr('transform', 'translate(' + margin.left + ',' + margin.bottom + ')')
    //   .attr('r', 3);
  }

  render() {
    return (<div style={svgContainer}>
      <svg ref={(node) => this.node = node}
        style={svgContent}
        viewBox={'0 0 '+ this.svgWidth +' '+ this.svgHeight}
        preserveAspectRatio='xMidYMid meet'/>
    </div>);
  }
}

ScatterPlotWindow.propTypes = {
  classes: PropTypes.object.isRequired,
};

export default withDSXContext(withStyles(styles)(ScatterPlotWindow));
