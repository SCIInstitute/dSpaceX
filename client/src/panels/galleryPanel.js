import React, { Component } from 'react';
import ExpansionPanel from '@material-ui/core/ExpansionPanel';
import ExpansionPanelDetails from '@material-ui/core/ExpansionPanelDetails'
import ExpansionPanelSummary from '@material-ui/core/ExpansionPanelSummary';
import ExpandMoreIcon from '@material-ui/icons/ExpandMore';
import Typography from '@material-ui/core/Typography';
import Histogram from './histogram';

class GalleryPanel extends Component {
  render() {

    return (
      <ExpansionPanel>
        <ExpansionPanelSummary expandIcon={<ExpandMoreIcon/>}/>
        <ExpansionPanelDetails>
          <Histogram key="histogram" size={[190, 100]}
                     data={[1,2,3]} />,
        </ExpansionPanelDetails>
      </ExpansionPanel>
    );
  }
}

export default GalleryPanel;