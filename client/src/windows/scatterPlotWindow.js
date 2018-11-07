import PropTypes from 'prop-types';
import React from 'react';
import * as d3 from "d3";
import {withDSXContext} from '../dsxContext.js';
import {withStyles} from '@material-ui/core/styles';


const styles = (theme) => ({
  root: {
    overflowX: 'auto',
  }
});

const SVGStyle = {
  width: "100%",
  height: "100%",
};

class ScatterPlotWindow extends React.Component {
  constructor(props) {
    super(props);

    this.client = this.props.dsxContext.client;
    this.state = {
      fields: [],
    };
  }

  async getParameters() {
    let {datasetId, parameterNames} = this.props.dataset;
    let parameters = [];
    let data = [];

    for (let i = 0; i < parameterNames.length; i++) {
      let parameterName = parameterNames[i];
      let {parameter} =
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
   * Calculates the qois for the given parameters
   */
   calculateQOIS(parameters) {
    let qois = [];
    let that = this;
    parameters.forEach(function (d, i) {
      let a = d["Major Axis"];
      let b = d["Minor Axis"];
      let calc_qoi = {
        "Area": that.area(a, b),
        "Perimeter": that.perimeter(a, b)
      };
      qois.push(calc_qoi);
    });
    return qois;
  }

  /**
   * Calculates area of ellipse
   */
  area(majorAxis, minorAxis) {
    return majorAxis * minorAxis * Math.PI;
  };

  /**
   * Calculates perimeter of ellipse
   */
  perimeter(majorAxis, minorAxis) {
    return Math.PI * (3*(majorAxis+minorAxis) - Math.sqrt((3*majorAxis+minorAxis)*(majorAxis+3*minorAxis)));
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

    // Remove stale chart
    d3.select(node).selectAll("*").remove();

    // Get bounding rect and create margins
    let boundingRect = node.getBoundingClientRect();
    let margin = {top: 30, right: 50, bottom: 40, left: 40};
    let padding = {x_small: 10, small: 15};
    let width = boundingRect.width - margin.left - margin.right;
    let height = boundingRect.height - margin.top - margin.bottom;

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
      .domain(d3.extent(data, function (d) {
        return d[columnNames[0]];
      }))
      .nice();

    let yScale = d3.scaleLinear()
      .range([height, 0])
      .domain(d3.extent(data, function (d) {
        return d[columnNames[1]];
      }))
      .nice();

    // Add axes
    let xAxis = d3.axisBottom(xScale);
    let yAxis = d3.axisLeft(yScale);
    d3.select(node)
      .append("g")
      .attr("transform", "translate(" + margin.left + "," + (height + margin.bottom) + ")")
      .call(xAxis);

    d3.select(node)
      .append("g")
      .attr("transform", "translate(" + margin.left + "," + margin.bottom + ")")
      .call(yAxis);

    // Add Labels
    d3.select(node).append("text")
      .attr("x", (margin.top + padding.small))
      .attr("y", (-1 * (margin.left + padding.x_small)))
      .attr("transform", "rotate(90)")
      .classed("label", true)
      .text(columnNames[0]);

    d3.select(node).append("text")
      .attr("x", (width + padding.small))
      .attr("y", (height + margin.bottom - padding.x_small))
      .attr("text-anchor", "end")
      .classed("label", true)
      .text(columnNames[1]);

    // Add data
    let circles = d3.select(node).append("g")
      .selectAll("circle")
      .data(data);
    let circlesEntering = circles.enter().append("circle");
    circles.exit().remove();
    circles = circles.merge(circlesEntering);

    circles
      .attr("cx", d => xScale(d[columnNames[0]]))
      .attr("cy", d => yScale(d[columnNames[1]]))
      .attr("transform", "translate(" + margin.left + "," + margin.bottom + ")")
      .attr("r", 5);

  }

  render() {
    return <svg ref={node => this.node = node} style={SVGStyle}></svg>
  }
}

ScatterPlotWindow.propTypes = {
  classes: PropTypes.object.isRequired,
};

export default withDSXContext(withStyles(styles)(ScatterPlotWindow));