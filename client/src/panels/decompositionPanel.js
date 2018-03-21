import ExpandMoreIcon from 'material-ui-icons/ExpandMore';
import ExpansionPanel from 'material-ui/ExpansionPanel';
import { ExpansionPanelSummary } from 'material-ui/ExpansionPanel';
import { ExpansionPanelDetails } from 'material-ui/ExpansionPanel';
import { FormControl } from 'material-ui/Form';
import { InputLabel } from 'material-ui/Input';
import List, { ListItem } from 'material-ui/List';
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
    this.handlePersistenceLevelChange =
        this.handlePersistenceLevelChange.bind(this);

    this.state = {
      decompositionMode: '',
      persistenceLevel: '',
      minPersistence: null,
      maxPersistence: null,
      crystals: [],
    };

    this.client = this.props.client;
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

    if (mode == 'Morse-Smale') {
      let k = 15;
      let datasetId = this.props.dataset.id;
      this.client.fetchMorseSmalePersistence(datasetId, k)
        .then(function(result) {
          this.setState({
            minPersistence: result.minPersistenceLevel,
            maxPersistence: result.maxPersistenceLevel,
          });
        }.bind(this));
    } else {
      this.setState({
        minPersistence: null,
        maxPersistence: null,
      });
    }
  }

  /**
   * Handles when the persistence level combo is changed.
   * @param {Event} event
   */
  handlePersistenceLevelChange(event) {
    let level = event.target.value;
    this.setState({
      persistenceLevel: level,
    });

    if (level != '') {
      let k = 15;
      let datasetId = this.props.dataset.id;
      let persistenceLevel = parseInt(level);
      this.client.fetch;
      this.client
        .fetchMorseSmalePersistenceLevel(datasetId, k, persistenceLevel)
        .then(function(result) {
          this.setState({
            crystals: result.complex.crystals,
          });
        }.bind(this));
    } else {
      this.setState({
        crystals: [],
      });
    }
  }

  /**
   * Renders the component to HTML.
   * @return {HTML}
   */
  render() {
    const { classes } = this.props;
    let persistenceLevels = [];
    if (this.state.minPersistence && this.state.maxPersistence) {
      for (let i=this.state.maxPersistence;
        i >= this.state.minPersistence; i--) {
        persistenceLevels.push(i);
      }
    }
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
            width: '100%', boxSizing: 'border-box' }}>
            <FormControl className={classes.formControl}
              style={{ width: '100%',
                boxSizing: 'border-box',
                paddingRight: '10px' }}>
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
            <div style={{ height:'15px' }}></div>
            <FormControl className={classes.formControl}
              style={{
                width: '100%',
                boxSizing: 'border-box',
                paddingRight: '10px',
              }}>
              <InputLabel htmlFor='persistence-field'>
                Persistence Level
              </InputLabel>
              <Select ref="persistenceCombo"
                style={{ width:'100%' }}
                value={this.state.persistenceLevel}
                onChange={this.handlePersistenceLevelChange}
                inputProps={{
                  name: 'persistence',
                  id: 'persistence-field',
                }}>
                <MenuItem value=''>
                  <em>None</em>
                </MenuItem>
                {
                  persistenceLevels.map((level) => (
                    <MenuItem value={'' + level}
                      key={level}
                      style={{ height:'20px' }}>
                      <em>{level}</em>
                    </MenuItem>
                  ))
                }
              </Select>
            </FormControl>
            <div style={{ height:'5px' }}></div>

            <List style={{ maxHeight:'200px', overflow:'auto' }}>
              {
                this.state.crystals.map((crystal, i) => (
                  <ListItem key={i} style={{ padding:'1px' }}>
                    <ExpansionPanel style={{ width:'100%' }}>
                      <ExpansionPanelSummary expandIcon={ <ExpandMoreIcon/> }>
                        <Typography>{'Crystal ' + (i+1)}</Typography>
                      </ExpansionPanelSummary>
                      <ExpansionPanelDetails>
                        <div>{'... '}</div>
                      </ExpansionPanelDetails>
                    </ExpansionPanel>
                  </ListItem>
                ))
              }

              {
                /*
              <ListItem button style={{ height: '40px' }}>
                <ListItemText primary="Crystal 2"/>
              </ListItem>
              <ListItem button style={{ height: '40px' }}>
                <ListItemText primary="Crystal 3"/>
              </ListItem>
              <ListItem button style={{ height: '40px' }}>
                <ListItemText primary="Crystal 4"/>
              </ListItem>
              <ListItem button style={{ height: '40px' }}>
                <ListItemText primary="Crystal 5"/>
              </ListItem>
               */
              }
            </List>
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
