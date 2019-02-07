import Input from '@material-ui/core/Input';
import React, { Component } from 'react';
import FormControl from '@material-ui/core/es/FormControl/FormControl';
import InputLabel from '@material-ui/core/InputLabel';
import MenuItem from '@material-ui/core/es/MenuItem/MenuItem';
import Paper from '@material-ui/core/Paper';
import Select from '@material-ui/core/Select';
import { withStyles } from '@material-ui/core/styles';

/**
 * The FilterPanel provides the drop downs
 * to select the parameter/qoi and to setup
 * the histogram to create a filter for the gallery view.
 */
class FilterPanel extends Component {
  /**
   * The FilterPanelConstructor
   * @param {object} props
   */
  constructor(props) {
    super(props);
    this.state = {
      attributeGroup: '',
    };

    this.handleAttributeChange = this.handleAttributeChange.bind(this);
  };

  /**
   * Handle attribute group change
   * @param {object} event
   */
  handleAttributeChange(event) {
    this.setState({ attributeGroup:event.target.value });
  };

  /**
   * Renders the filter panel
   * @return {jsx}
   */
  render() {
    const { classes } = this.props;
    return (
      <div style={{ width:'100%' }}>
        <FormControl className={classes.formControl} style={{ display:'flex', wrap:'nowrap' }}>
          <InputLabel htmlFor='filter-attribute-label'>Attribute Group</InputLabel>
          <Select
            value={this.state.attributeGroup}
            onChange={this.handleAttributeChange}
            input={<Input name='attributeGroup' id='filter-attribute-label'/>}
            displayEmpty
            name='attributeGroup'
            autoWidth={true}>
            <MenuItem value='parameters'>Parameters</MenuItem>
            <MenuItem value='qois'>Qois</MenuItem>
          </Select>
        </FormControl>
      </div>);
  }
}

export default withStyles({})(FilterPanel);
