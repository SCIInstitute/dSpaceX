import ExpandMoreIcon from 'material-ui-icons/ExpandMore';
import ExpansionPanel from 'material-ui/ExpansionPanel';
import { ExpansionPanelSummary } from 'material-ui/ExpansionPanel';
import { ExpansionPanelDetails } from 'material-ui/ExpansionPanel';
import React from 'react';
import Typography from 'material-ui/Typography';

/**
 * The DisplayPanel component provides a user a means to adjust
 * various global view settings from the side drawer.
 */
class DisplayPanel extends React.Component {
  /**
   * DisplayPanel constructor.
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
        <ExpansionPanelSummary expandIcon={<ExpandMoreIcon />}>
          <Typography>Display</Typography>
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

export default DisplayPanel;
