import React, { Component } from 'react';
import Button from '@material-ui/core/Button';
import Drawer from '@material-ui/core/Drawer';
import FilterOptions from '../filterOptions';
import { withStyles } from '@material-ui/core/styles';

const drawerWidth = 230;
const styles = (theme) => ({
  toolbar: theme.mixins.toolbar,
  drawerPaper: {
    width: drawerWidth,
    overflowX: 'hidden',
  },
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
    const { classes, filters, addFilter, updateFilter, removeFilter, qois, parameters } = this.props;
    return (
      <Drawer open={true} PaperProps={{ elevation:6 }} variant='persistent'
        classes={{ paper:classes.drawerPaper }} anchor='right'>
        { /* Add div to account for menu bar */ }
        <div className={classes.toolbar} />
        <Button color="primary" className={classes.button}
          onClick={() => addFilter(this.getId())}>
          Add Filter
        </Button>
        {filters.sort((a, b) => a.id - b.id).map((filterConfig, i) => {
          return <FilterOptions
            key={i}
            filterCount={filters.length}
            filterConfig={filterConfig}
            parameters={parameters}
            qois={qois}
            updateFilter={updateFilter}
            removeFilter={removeFilter}/>;
        })}
      </Drawer>);
  }
}

export default withStyles(styles)(FilterPanel);
