import ExpandMoreIcon from 'material-ui-icons/ExpandMore';
import ExpansionPanel from 'material-ui/ExpansionPanel';
import { ExpansionPanelSummary } from 'material-ui/ExpansionPanel';
import { ExpansionPanelDetails } from 'material-ui/ExpansionPanel';
import { FormControl } from 'material-ui/Form';
import { InputLabel } from 'material-ui/Input';
import { MenuItem } from 'material-ui/Menu';
import PropTypes from 'prop-types';
import React from 'react';
import Select from 'material-ui/Select';
import Typography from 'material-ui/Typography';
import { withStyles } from 'material-ui/styles';

/**
 * The Decomposition Panel component provides a display of the
 * Morse-Smale/ShapeOdds decomposition of the dataset.
 */
class DecompositionPanel extends React.Component {
  /**
   * DecompositionPanel constructor.
   * @param {object} props
   */
  constructor(props) {
    super(props);

    this.handleDecompositionModeChange =
        this.handleDecompositionModeChange.bind(this);

    this.state = {
      decompositionMode: 'Morse-Smale',
    };
  }

  /**
   * Handles when the decomposition combo is changed.
   * @param {Event} event
   */
  handleDecompositionModeChange(event) {
    let mode = event.target.value;
    this.setState({
      decompositionMode: mode,
    });
  }

  /**
   * Renders the component to HTML.
   * @return {HTML}
   */
  render() {
    const { classes } = this.props;
    return (
      // TODO: set disabled only when there's no case data.
      <ExpansionPanel disabled={!this.props.dataset}
        style={{ paddingLeft:'0px', margin:'1px' }}>
        <ExpansionPanelSummary expandIcon={ <ExpandMoreIcon/> }>
          <Typography>Decomposition</Typography>
        </ExpansionPanelSummary>
        <ExpansionPanelDetails style={{ paddingLeft: '15px',
          paddingRight: '10px', margin: '1px', width: '100%',
          boxSizing: 'border-box' }}>
          <div style={{ display: 'flex', flexDirection: 'column',
            width: '100%', boxSizing: 'border-box', paddingRight: '10px' }}>
            <FormControl className={classes.formControl}
              style={{ width:'100%', boxSizing:'border-box' }}>
              <InputLabel htmlFor='mode-field'>Mode</InputLabel>
              <Select ref="decompositionCombo"
                disabled={!this.props.dataset}
                value={this.state.decompositionMode}
                style={{ width:'100%' }}
                onChange={this.handleDecompositionModeChange}
                inputProps={{
                  name: 'mode',
                  id: 'mode-field',
                }}>
                <MenuItem value=''>
                  <em>None</em>
                </MenuItem>
                <MenuItem value='Morse-Smale'>
                  <em>Morse-Smale</em>
                </MenuItem>
                <MenuItem value='Shape-Odds'>
                  <em>Shape-Odds</em>
                </MenuItem>

              </Select>
            </FormControl>
          </div>
        </ExpansionPanelDetails>
      </ExpansionPanel>
    );
  }
}

// Enforce that Application receives styling.
DecompositionPanel.propTypes = {
  classes: PropTypes.object.isRequired,
};

// Wrap Application in Styling Container.
export default withStyles({})(DecompositionPanel);
