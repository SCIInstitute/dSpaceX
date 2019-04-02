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
  },
  table: {},
  tableWrapper: {},
});

const CustomTableRow = withStyles((theme) => ({
  selected: {
    backgroundColor: 'rgb(201, 220, 252)',
    border: 'solid thin',
    borderColor: '#ff3d00',
  },
}))(TableRow);

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

  /**
   * Fetches the tabular data for qois to render in the table.
   * @return {Promise}
   */
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

  /**
   *
   */
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
   * Renders the component to HTML.
   * @return {HTML}
   */
  render() {
    const { classes, selectedDesigns } = this.props;

    let columnNames = [];
    if (this.props.attributeGroup === 'parameters') {
      columnNames = this.props.dataset.parameterNames;
    } else if (this.props.attributeGroup === 'qois') {
      columnNames = this.props.dataset.qoiNames;
    }

    return (
      <Paper className={classes.root}>
        <Table className={classes.table}>
          <TableHead>
            <TableRow>
              <TableCell numeric padding='dense'>id</TableCell>
              {
                columnNames.map((n) => {
                  return (
                    <TableCell key={n} numeric padding='dense'>{n}</TableCell>
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
                    <CustomTableRow hover className={classes.row} key={i}
                      selected={selectedDesigns.has(i)}>
                      <TableCell numeric padding='dense'>{i}</TableCell>
                      {
                        columnNames.map((p) => {
                          return (
                            <TableCell numeric padding='dense'
                              key={i + '-' + p}>
                              { n[p] && n[p].toPrecision(5) }
                            </TableCell>
                          );
                        })
                      }
                    </CustomTableRow>
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
