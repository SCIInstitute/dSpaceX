import Paper from '@material-ui/core/Paper';
import PropTypes from 'prop-types';
import React from 'react';
import Table from '@material-ui/core/Table';
import TableBody from '@material-ui/core/TableBody';
import TableCell from '@material-ui/core/TableCell';
import TableHead from '@material-ui/core/TableHead';
import TableRow from '@material-ui/core/TableRow';
import { withDSXContext } from '../dsxContext.js';
import { withStyles } from '@material-ui/core/styles';

const styles = (theme) => ({
  root: {
    overflowX: 'auto',
    border: '1px solid gray',
  },
  table: {},
  tableWrapper: {},
  selected: {
    backgroundColor: '#c5cae8',
  },
  active: {
    backgroundColor: '#D3D3D3',
  },
});

/**
 * A Window Component for displaying tabular data.
 */
class TableWindow extends React.Component {
  /**
   * TableWindow constructor.
   * @param {object} props
   */
  constructor(props) {
    super(props);

    this.client = this.props.dsxContext.client;
    this.state = {
      fields: [],
      focusRow: null,
    };
  }

  /**
   * Fetches the tabular data for parameters to render in the table.
   * @return {Promise}
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
   * Fetches the tabular data for qois to render in the table.
   * @return {Promise}
   */
  async getQois() {
    let { datasetId, qoiNames } = this.props.dataset;
    let qois = [];
    let data = [];

    for (let i = 0; i < qoiNames.length; i++) {
      let qoiName = qoiNames[i];
      let { qoi } =
        await this.client.fetchQoi(datasetId, qoiName);
      qois.push(qoi);
    }

    let sampleCount = qois[0].length;

    for (let i = 0; i < sampleCount; i++) {
      let row = {};
      for (let j = 0; j < qoiNames.length; j++) {
        row[qoiNames[j]] = qois[j][i];
      }
      data.push(row);
    }

    return data;
  }

  /**
   *
   */
  componentDidMount() {
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

  /**
   * @param {object} prevProps
   */
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

  /**
   * Get the right class name for the table row
   * @param {number} id
   * @return {Object} class name
   */
  getClassName(id) {
    const { classes, selectedDesigns, activeDesigns } = this.props;
    const { numberOfSamples } = this.props.dataset;
    if (selectedDesigns.has(id)) {
      return classes.selected;
    } else if (activeDesigns.has(id)) {
      if (activeDesigns.size === numberOfSamples) {
        return classes.row;
      }
      return classes.active;
    }
    return classes.row;
  }

  /**
   * Renders the component to HTML.
   * @return {HTML}
   */
  render() {
    const { classes } = this.props;

    let columnNames = [];
    if (this.props.attributeGroup === 'parameters') {
      columnNames = this.props.dataset.parameterNames;
    } else if (this.props.attributeGroup === 'qois') {
      columnNames = this.props.dataset.qoiNames;
    }

    return (
      <Paper className={classes.root}>
        <Table className={classes.table}>
          <TableHead style={{ borderBottom:'1px solid black' }}>
            <TableRow>
              {columnNames.length > 0 && <TableCell
                style={{ fontWeight:'bold', fontSize:'medium' }} align='right' padding='dense'>id</TableCell>}
              {
                columnNames.map((n) => {
                  return (
                    <TableCell
                      style={{ fontWeight:'bold', fontSize:'medium' }} key={n} align='right' padding='dense'>{n}</TableCell>
                  );
                })
              }
            </TableRow>
          </TableHead>
          <TableBody>
            {
              this.state.fields ?
                this.state.fields.map((n, i) => {
                  return (
                    <TableRow
                      key={i}
                      className={this.getClassName(i)}
                      onClick={(e) => this.props.onDesignSelection(e, i)}>
                      <TableCell align='right' padding='dense'>{i}</TableCell>
                      {
                        columnNames.map((p) => {
                          return (
                            <TableCell align='right' padding='dense'
                              key={i + '-' + p}>
                              { n[p] && n[p].toPrecision(5) }
                            </TableCell>
                          );
                        })
                      }
                    </TableRow>
                  );
                }) : []
            }
          </TableBody>
        </Table>
      </Paper>
    );
  }
}

TableWindow.propTypes = {
  classes: PropTypes.object.isRequired,
};

export default withDSXContext(withStyles(styles)(TableWindow));
