
import PropTypes from 'prop-types';
import React from 'react';
import { withDSXContext } from '../dsxContext.js';
import { withStyles } from '@material-ui/core/styles';
import * as d3 from "d3";

const styles = (theme) => ({
  root: {
    overflowX: 'auto',
  }
});

class ScatterPlotWindow extends React.Component {
  constructor(props) {
    super(props);

    this.client = this.props.dsxContext.client;
    this.state = {
      fields: [],
    };
  }

  async getParameters() {
    let { datasetId, parameterNames } = this.props.dataset;
    let parameters = [];
    let data = [];

    for (let i=0; i < parameterNames.length; i++) {
      let parameterName = parameterNames[i];
      let { parameter } =
        await this.client.fetchParameter(datasetId, parameterName);
      parameters.push(parameter);
    }

    let sampleCount = parameters[0] ? parameters[0].length : 0;
    for (let i=0; i < sampleCount; i++) {
      let row = {};
      for (let j=0; j < parameterNames.length; j++) {
        row[parameterNames[j]] = parameters[j][i];
      }
      data.push(row);
    }

    return data;
  }

  async getQois() {
    let { datasetId, qoiNames } = this.props.dataset;
    let qois = [];
    let data = [];

    for (let i=0; i < qoiNames.length; i++) {
      let qoiName = qoiNames[i];
      let { qoi } =
        await this.client.fetchQoi(datasetId, qoiName);
      qois.push(qoi);
    }

    let sampleCount = qois[0].length;

    for (let i=0; i < sampleCount; i++) {
      let row = {};
      for (let j=0; j < qoiNames.length; j++) {
        row[qoiNames[j]] = qois[j][i];
      }
      data.push(row);
    }

    return data;
  }

  componentWillMount() {
    switch (this.props.attributeGroup) {
      case 'parameters':
        this.getParameters().then((data) => {
          this.setState({
            fields: data,
          });
        });
        break;
      case 'qois':
        this.getQois().then((data) => {
          this.setState({
            fields: data,
          });
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
            this.setState({
              fields: data,
            });
          });
          break;
        case 'qois':
          this.getQois().then((data) => {
            this.setState({
              fields: data,
            });
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

  render() {
    const {classes} = this.props;

    const xScale = d3.scaleLinear()
      .range([0, this.props.width])
      .domain(d3.extent(this.state.fields, function (d) {
        return d[0];
      }))
      .nice();

    let yScale = d3.scaleLinear()
      .range([this.props.height, 0])
      .domain(d3.extent(this.state.fields, function (d) {
        return d[1];
      }))
      .nice();

    const test = this.state.fields;
    const circles = this.state.fields.map((d, i) => <circle key={i} cx={xScale(d[0])} cy={yScale(d[1])} r={5}></circle>);

    return <svg width={500} height={500}>
      {circles}
    </svg>
  }
}

ScatterPlotWindow.propTypes = {
  classes: PropTypes.object.isRequired,
};

export default withDSXContext(withStyles(styles)(ScatterPlotWindow));