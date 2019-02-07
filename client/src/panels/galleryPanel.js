import React, { Component } from 'react';
import { AddCircle } from '@material-ui/icons';
import Button from '@material-ui/core/Button/Button';
import ExpandMoreIcon from '@material-ui/icons/ExpandMore';
import ExpansionPanel from '@material-ui/core/ExpansionPanel';
import ExpansionPanelDetails from '@material-ui/core/ExpansionPanelDetails';
import ExpansionPanelSummary from '@material-ui/core/ExpansionPanelSummary';
import FilterPanel from './filterPanel';
import Icon from '@material-ui/core/Icon';
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
    this.state = {
      filters: [],
    };

    this.addFilter = this.addFilter.bind(this);
  };

  /**
   * Handles when a user selects the add filter button
   */
  addFilter() {
    let filterConfig = {
      id: this.state.filters.length,
    };
    this.setState({
      filters: this.state.filters.concat(filterConfig),
    });
  };

  /**
   * Renders the gallery panel
   * @return {jsx}
   */
  render() {
    const { classes } = this.props;
    return (
      <ExpansionPanel defaultExpanded={true}>
        <ExpansionPanelSummary expandIcon={<ExpandMoreIcon/>}/>
        <ExpansionPanelDetails>
          {this.state.filters.map((filterConfig, i) => {
            return <FilterPanel
              key={i}
              filterConfig={filterConfig}
              parameters={this.props.parameters}
              qois={this.props.qois}/>;
          })}
          <IconButton variant='raised' onClick={this.addFilter}>
            <AddCircle className={classes.icon} color='disabled' fontSize='large'/>
          </IconButton>
        </ExpansionPanelDetails>
      </ExpansionPanel>
    );
  }
}

export default withStyles({})(GalleryPanel);
