import { FormControl, FormHelperText } from 'material-ui/Form';
import Input, { InputLabel } from 'material-ui/Input';
import { MenuItem } from 'material-ui/Menu';
import React from 'react';
import Paper from 'material-ui/Paper';
import PropTypes from 'prop-types';
import Select from 'material-ui/Select';
import { withStyles } from 'material-ui/styles';


const datasetList = [ 'Colorado', 'Sandia' ];

class DatasetPanel extends React.Component {
  constructor(props) {
    super(props);
    this.state = {
      dataset: ''
    };

    this.handleChange = this.handleChange.bind(this);
  }  

  handleChange(event) {
    this.setState({ [event.target.name]: event.target.value });
  };

  render() {
    const { classes } = this.props;
    return (
      <Paper style={{padding: '15px', paddingBottom: '50px'}}>
        <div style={{display: 'flex',  flexDirection:'column'}}>
          <FormControl className={classes.formControl}>
            <InputLabel htmlFor="dataset-field">Dataset</InputLabel>
            <Select value={this.state.dataset}
                onChange={this.handleChange}
                inputProps={{
                  name: 'dataset',
                  id: 'dataset-field',
                }}
              >
              <MenuItem value="">
                <em>None</em>
              </MenuItem>
              {
                datasetList.map(dataset => (
                  <MenuItem value={dataset} key={dataset}>{dataset}</MenuItem>  
                ))
              }
            </Select>            
          </FormControl>              
        </div>
      </Paper>        
    );
  }
}

// Enforce that Application receives styling.
DatasetPanel.propTypes = {
  classes: PropTypes.object.isRequired,
};

// Wrap Application in Styling Container.
export default withStyles({})(DatasetPanel);
