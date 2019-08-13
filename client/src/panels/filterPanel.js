import React, { Component } from 'react';
import Button from '@material-ui/core/Button';
import Drawer from '@material-ui/core/Drawer';
import FilterOptions from '../filterOptions';
import Grid from '@material-ui/core/Grid';
import Switch from '@material-ui/core/Switch';
import Typography from '@material-ui/core/Typography';
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
    const { classes, union, changeFilterOperation,
      filters, addFilter, updateFilter, removeFilter, qois, parameters } = this.props;
    return (
      <Drawer open={true} PaperProps={{ elevation:6 }} variant='persistent'
        classes={{ paper:classes.drawerPaper }} anchor='right'>
        { /* Add div to account for menu bar */ }
        <div className={classes.toolbar} />
        <Typography component="div" style={{ marginLeft:'25px', marginTop:'5px' }}>
          <Grid component="label" container alignItems="center" spacing={0}>
            <Grid item>INTERSECT</Grid>
            <Grid item>
              <Switch
                color="primary"
                checked={union}
                onChange={changeFilterOperation}
                value="union"
              />
            </Grid>
            <Grid item>UNION</Grid>
          </Grid>
        </Typography>
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
