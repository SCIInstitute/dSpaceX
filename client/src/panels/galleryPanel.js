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
    this.state = {
      filters: [],
    };

    this.addFilterPanel = this.addFilterPanel.bind(this);
  };

  /**
   * Handles when a user selects the add filter button
   */
  addFilterPanel() {
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
              id={i}
              filterConfig={filterConfig}
              parameters={this.props.parameters}
              qois={this.props.qois}
              addFilter={this.props.addFilter}/>;
          })}
          <div style={{ width:'50px', height:'50px' }}>
            <IconButton variant='raised' onClick={this.addFilterPanel}>
              <AddCircle className={classes.icon} color='disabled' fontSize='large'/>
            </IconButton>
          </div>
        </ExpansionPanelDetails>
      </ExpansionPanel>
    );
  }
}

export default withStyles({})(GalleryPanel);
