import ExpandMoreIcon from 'material-ui-icons/ExpandMore';
import ExpansionPanel from 'material-ui/ExpansionPanel';
import { ExpansionPanelSummary } from 'material-ui/ExpansionPanel';
import { ExpansionPanelDetails } from 'material-ui/ExpansionPanel';
import React from 'react';
import Typography from 'material-ui/Typography';

/**
 * Component for showing data set case details.
 */
class CasesPanel extends React.Component {
  /**
   * CasesPanel constructor.
   * @param {object} props
   */
  constructor(props) {
    super(props);
  }

  /**
   * Renders the component to HTML.
   * @return {HTML}
   */
  render() {
    return (
      // TODO: set disabled only when there's no case data.
      <ExpansionPanel style={{ margin:'1px' }} disabled={true}>
        <ExpansionPanelSummary expandIcon={ <ExpandMoreIcon /> }>
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
