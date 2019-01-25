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
  'verticalAlight': 'top',
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

    this.svgWidth = 1000;
    this.svgHeight = 500;
    this.viewBoxWidth = 1000;
    this.viewBoxHeight = 500;

    this.client = this.props.dsxContext.client;
    this.state = {
      fields: [],
    };
  }

  /**
   * This gets the parameters for the dataset
   * @return {Promise<Array>}
   */
  async getParameters() {
    let { datasetId, parameterNames } = this.props.dataset;
    let parameters = [];
    let data = [];

    for (let i = 0; i < parameterNames.length; i++) {
      let parameterName = parameterNames[i];
      let { parameter } =
        await this.client.fetchParameter(datasetId, parameterName);
      parameters.push(parameter);
    }

    let sampleCount = parameters[0] ? parameters[0].length : 0;
    for (let i = 0; i < sampleCount; i++) {
      let row = {};
      for (let j = 0; j < parameterNames.length; j++) {
        row[parameterNames[j]] = parameters[j][i];
      }
      data.push(row);
    }
    return data;
  }

  /**
   * Calculates the QOIs for the dataset
   * @return {Array}
   * @param {Array} parameters
   */
  calculateQOIS(parameters) {
    let qois = [];
    let that = this;
    parameters.forEach(function(d, i) {
      let a = d['Major Axis'];
      let b = d['Minor Axis'];
      let calcQOI = {
        'Area': that.area(a, b),
        'Perimeter': that.perimeter(a, b),
      };
      qois.push(calcQOI);
    });
    return qois;
  }

  /**
   * Calculates area given major and minor axis
   * @param {double} majorAxis
   * @param {double} minorAxis
   * @return {number}
   */
  area(majorAxis, minorAxis) {
    return majorAxis * minorAxis * Math.PI;
  };

  /**
   * Calculates perimeter given major and minor axis
   * @param {double} majorAxis
   * @param {double} minorAxis
   * @return {number}
   */
  perimeter(majorAxis, minorAxis) {
    return Math.PI
      * (3 * (majorAxis + minorAxis) - Math.sqrt((3 * majorAxis + minorAxis)
        * (majorAxis + 3 * minorAxis)));
  };

  componentWillMount() {
    switch (this.props.attributeGroup) {
      case 'parameters':
        this.getParameters().then((data) => {
          this.drawChart(data);
        });
        break;
      case 'qois':
        this.getParameters().then((data) => {
          this.drawChart(this.calculateQOIS(data));
        });
        break;
      default:
        this.setState({
          fields: [],
        });
        break;
    }
  }

  componentDidUpdate(prevProps) {
    if (this.props.dataset !== prevProps.dataset ||
      this.props.attributeGroup !== prevProps.attributeGroup) {
      switch (this.props.attributeGroup) {
        case 'parameters':
          this.getParameters().then((data) => {
            this.drawChart(data);
          });
          break;
        case 'qois':
          this.getParameters().then((data) => {
            this.drawChart(this.calculateQOIS(data));
          });
          break;
        default:
          this.setState({
            fields: [],
          });
          break;
      }
    }
  }

  drawChart(data) {
    // Get this node
    const node = this.node;
    d3.select(node).selectAll('*').remove();

    // Create margins
    let margin = { top: 30, right: 50, bottom: 40, left: 40 };
    let padding = { x_small: 10, small: 5 };
    let width = this.svgWidth - margin.left - margin.right;
    let height = this.svgHeight - margin.top - margin.bottom;

    // Get column names
    let columnNames = [];
    if (this.props.attributeGroup === 'parameters') {
      columnNames = this.props.dataset.parameterNames;
    } else if (this.props.attributeGroup === 'qois') {
      columnNames = ['Area', 'Perimeter'];
    }

    // Create scales
    let xScale = d3.scaleLinear()
      .range([0, width])
      .domain(d3.extent(data, function(d) {
        return d[columnNames[0]];
      }))
      .nice();

    let yScale = d3.scaleLinear()
      .range([height, 0])
      .domain(d3.extent(data, function(d) {
        return d[columnNames[1]];
      }))
      .nice();

    // Add axes
    let xAxis = d3.axisBottom(xScale)
      .tickFormat(d3.format('.0s'));
    let yAxis = d3.axisLeft(yScale);
    d3.select(node)
      .append('g')
      .attr('transform',
        'translate(' + margin.left + ',' + (height + margin.bottom) + ')')
      .call(xAxis);

    d3.select(node)
      .append('g')
      .attr('transform',
        'translate(' + margin.left + ',' + margin.bottom + ')')
      .call(yAxis);

    // Add Labels
    d3.select(node).append('text')
      .attr('x', (margin.top + padding.small))
      .attr('y', (-1 * (margin.left + padding.x_small)))
      .attr('transform', 'rotate(90)')
      .text(columnNames[0]);

    d3.select(node).append('text')
      .attr('x', (width + padding.small))
      .attr('y', (height + margin.bottom - padding.x_small))
      .attr('text-anchor', 'end')
      .text(columnNames[1]);

    // Add data
    let circles = d3.select(node).append('g')
      .selectAll('circle')
      .data(data);
    let circlesEntering = circles.enter().append('circle');
    circles.exit().remove();
    circles = circles.merge(circlesEntering);

    circles
      .attr('cx', (d) => xScale(d[columnNames[0]]))
      .attr('cy', (d) => yScale(d[columnNames[1]]))
      .attr('transform', 'translate(' + margin.left + ',' + margin.bottom + ')')
      .attr('r', 3);
  }

  render() {
    return (<div style={svgContainer}>
      <svg ref={(node) => this.node = node} style={svgContent}
           viewBox={'0 0 ' + this.viewBoxWidth + ' ' + this.viewBoxHeight}
           preserveAspectRatio={'xMinYMid meet'}/>
    </div>);
  }
}

ScatterPlotWindow.propTypes = {
  classes: PropTypes.object.isRequired,
};

export default withDSXContext(withStyles(styles)(ScatterPlotWindow));
