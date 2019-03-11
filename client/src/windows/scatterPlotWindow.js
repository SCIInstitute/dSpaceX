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
    this.svgHeight = 500;

    this.state = {
      parameters: [],
      qois: [],
      fields: [],
    };

    this.getParameters = this.getParameters.bind(this);
    this.getQois = this.getQois.bind(this);
    this.drawChart = this.drawChart.bind(this);
    this.compareConfigs = this.compareConfigs.bind(this);
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
   * This compare two configs to see if
   * the graph needs to be updated
   * @param prevConfig
   * @param currentConfig
   * @returns {boolean}
   */
  compareConfigs(prevConfig, currentConfig) {
    if (prevConfig.xAttributeGroup !== currentConfig.xAttributeGroup) {
      return false;
    }
    if (prevConfig.xAttribute !== currentConfig.xAttribute) {
      return false;
    }
    if (prevConfig.yAttributeGroup !== currentConfig.yAttributeGroup) {
      return false;
    }
    if (prevConfig.yAttribute !== currentConfig.yAttribute) {
      return false;
    }
    if (prevConfig.markerAttributeGroup !== currentConfig.markerAttributeGroup) {
      return false;
    }
    if (prevConfig.markerAttribute !== currentConfig.markerAttribute) {
      return false;
    }
    return true;
  }

  componentWillMount() {
    this.getParameters().then((data) => {
      this.setState({ parameters:data } );
    });
    this.getQois().then((data) => {
      this.setState({ qois:data } );
    });
    this.drawChart();
  }

  componentDidUpdate(prevProps) {
    if (this.props.dataset !== prevProps.dataset || !this.compareConfigs(prevProps.config, this.props.config)) {
      switch (this.props.config.xAttributeGroup) {
        case 'parameters':
          this.getParameters().then((data) => {
            this.setState({ parameters:data } );
          });
          break;
        case 'qois':
          this.getQois().then((data) => {
            this.setState({ qois:data } );
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
    let margin = { top:30, right:50, bottom:40, left:40 };
    let padding = { x_small:10, small:5 };
    let width = this.svgWidth - margin.left - margin.right;
    let height = this.svgHeight - margin.top - margin.bottom;


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
      <svg ref={(node) => this.node = node}
        style={svgContent}
        viewBox={'0 0 1000 500'}
        preserveAspectRatio={'xMinYMid meet'}/>
    </div>);
  }
}

ScatterPlotWindow.propTypes = {
  classes: PropTypes.object.isRequired,
};

export default withDSXContext(withStyles(styles)(ScatterPlotWindow));
