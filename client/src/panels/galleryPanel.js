import React, { Component } from 'react';
import { AddCircle } from '@material-ui/icons';
import ExpandMoreIcon from '@material-ui/icons/ExpandMore';
import ExpansionPanel from '@material-ui/core/ExpansionPanel';
import ExpansionPanelDetails from '@material-ui/core/ExpansionPanelDetails';
import ExpansionPanelSummary from '@material-ui/core/ExpansionPanelSummary';
import FilterPanel from './filterPanel';
import IconButton from '@material-ui/core/es/IconButton/IconButton';
import { withStyles } from '@material-ui/core/styles';

/**
 * The GalleryPanel provides the interactivity and ability
 * to filter the Gallery View of the thumbnails for a design
 */
class GalleryPanel extends Component {
  /**
   * Constructs the GalleryPanel object
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
   * Renders the gallery panel
   * @return {jsx}
   */
  render() {
    const { classes, filters, addFilter } = this.props;
    return (
      <ExpansionPanel defaultExpanded={false}>
        <ExpansionPanelSummary expandIcon={<ExpandMoreIcon/>}/>
        <ExpansionPanelDetails style={{ overflow:'auto hidden' }}>
          <div style={{ width:'50px', height:'50px', marginRight:'10px' }}>
            <IconButton variant='raised' onClick={() => addFilter(this.getId())}>
              <AddCircle className={classes.icon} color='disabled' fontSize='large'/>
            </IconButton>
          </div>
          {filters.sort((a, b) => b.id - a.id).map((filterConfig, i) => {
            return <FilterPanel
              key={i}
              filterCount={filters.length}
              filterConfig={filterConfig}
              parameters={this.props.parameters}
              qois={this.props.qois}
              updateFilter={this.props.updateFilter}
              removeFilter={this.props.removeFilter}/>;
          })}
        </ExpansionPanelDetails>
      </ExpansionPanel>
    );
  }
}

export default withStyles({})(GalleryPanel);
