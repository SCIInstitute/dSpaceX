import React, { Component } from 'react';
import { AddCircle } from '@material-ui/icons';
import Drawer from '@material-ui/core/Drawer';
import FilterOptions from '../filterOptions';
import IconButton from '@material-ui/core/es/IconButton/IconButton';
import { withStyles } from '@material-ui/core/styles';

const styles = (theme) => ({
  toolbar: theme.mixins.toolbar,
});

/**
 * The FilterPanel provides the interactivity and ability
 * to filter the Gallery View of the thumbnails for a design
 */
class FilterPanel extends Component {
  /**
   * Constructs the FilterPanel object
   * @param {object} props
   */
  constructor(props) {
    super(props);
  };

  /**
   * Need to make sure that a unique id is assigned
   * to each filter
   * @return {number} unique identifier for filter
   */
  getId() {
    const { filters } = this.props;
    if (filters.length === 0) {
      return 1;
    } else {
      let ids = [];
      filters.forEach((f) => {
        ids.push(f.id);
      });
      return Math.max(...ids) + 1;
    }
  }

  /**
   * Renders the filter panel
   * @return {jsx}
   */
  render() {
    const { classes, filters, addFilter } = this.props;
    return (
      <Drawer open={true} PaperProps={{ elevation:6 }} variant='persistent'
        classes={{ paper:classes.drawerPaper }} anchor='right'>
        { /* Add div to account for menu bar */ }
        <div className={classes.toolbar} />
        <div style={{ width:'50px', height:'50px' }}>
          <IconButton variant='raised' onClick={() => addFilter(this.getId())}>
            <AddCircle className={classes.icon} color='disabled' fontSize='large'/>
          </IconButton>
        </div>
        {filters.sort((a, b) => b.id - a.id).map((filterConfig, i) => {
          return <FilterOptions
            key={i}
            filterCount={filters.length}
            filterConfig={filterConfig}
            parameters={this.props.parameters}
            qois={this.props.qois}
            updateFilter={this.props.updateFilter}
            removeFilter={this.props.removeFilter}/>;
        })}
      </Drawer>);
  }
}

export default withStyles(styles)(FilterPanel);
