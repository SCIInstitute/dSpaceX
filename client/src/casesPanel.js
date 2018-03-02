import ExpandMoreIcon from 'material-ui-icons/ExpandMore';
import ExpansionPanel from 'material-ui/ExpansionPanel';
import { ExpansionPanelSummary } from 'material-ui/ExpansionPanel';
import { ExpansionPanelDetails } from 'material-ui/ExpansionPanel';
import React from 'react';
import Typography from 'material-ui/Typography';

class CasesPanel extends React.Component {
  constructor(props) {
    super(props);    
  }  

  render() {
    const { classes } = this.props;
    return (
      <ExpansionPanel style={{margin: '1px'}}>
        <ExpansionPanelSummary expandIcon={<ExpandMoreIcon />}>
          <Typography>Cases</Typography>
        </ExpansionPanelSummary>
        <ExpansionPanelDetails>
          <Typography>
            ...
          </Typography>
        </ExpansionPanelDetails>
      </ExpansionPanel>     
    );
  }
}

export default CasesPanel;
