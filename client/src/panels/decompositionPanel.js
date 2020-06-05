import Checkbox from '@material-ui/core/Checkbox';
import ExpandMoreIcon from '@material-ui/icons/ExpandMore';
import ExpansionPanel from '@material-ui/core/ExpansionPanel';
import ExpansionPanelDetails from '@material-ui/core/ExpansionPanelDetails';
import ExpansionPanelSummary from '@material-ui/core/ExpansionPanelSummary';
import FormControl from '@material-ui/core/FormControl';
import Histogram from './histogram';
import InputLabel from '@material-ui/core/InputLabel';
import List from '@material-ui/core/List';
import ListItem from '@material-ui/core/ListItem';
import ListItemText from '@material-ui/core/ListItemText';
import MenuItem from '@material-ui/core/MenuItem';
import PropTypes from 'prop-types';
import React from 'react';
import Select from '@material-ui/core/Select';
import TextField from '@material-ui/core/TextField';
import Typography from '@material-ui/core/Typography';
import { withDSXContext } from '../dsxContext.js';
import { withStyles } from '@material-ui/core/styles';
import {Button} from "@material-ui/core";

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
    this.handleDecompositionCategoryChange =
        this.handleDecompositionCategoryChange.bind(this);
    this.handleDecompositionFieldChange =
        this.handleDecompositionFieldChange.bind(this);
    this.handlePersistenceLevelChange =
        this.handlePersistenceLevelChange.bind(this);
    this.handleSliderChange = this.handleSliderChange.bind(this);
    this.handleSliderRelease = this.handleSliderRelease.bind(this);
    this.handleCrystalToggle = this.handleCrystalToggle.bind(this);
    this._getDecompositionFieldMenuItems =
        this._getDecompositionFieldMenuItems.bind(this);
    this.decompositionConfigValid = this.decompositionConfigValid.bind(this);
    this.fetchDecomposition = this.fetchDecomposition.bind(this);
    this.handleNearestNeighborChange = this.handleNearestNeighborChange.bind(this);

    this.state = {
      devMode: false,

      datasetId: this.props.dataset.datasetId,

      interpolationModel: 'None',
      model: {
        sigma: 0.15,
      },

      decompositionMode: 'Morse-Smale',
      ms: {
        knn: 15,
        persistence: -1,
        noise: false,
        sigma: 0.25,
        smoothing: 15.0,
      },

      decompositionCategory: 'qoi',
      decompositionField: this.props.dataset.qoiNames[0],
      persistenceLevel: '',
      minPersistence: null,
      maxPersistence: null,
      complexSizes: [],
      crystals: [],
      sliderPersistence: null,
    };

    this.client = this.props.dsxContext.client;
  }

  /**
   * Returns true if the configuration set is valid.
   * @param {object} config
   * @return {boolean}
   */
  decompositionConfigValid(config) {
    // Currently only Morse-Smale is supported.
    if (config.decompositionMode != 'Morse-Smale') {
      if (config.decompositionMode == 'Shared-GP') {
        console.log('validating Shared-GP decomposition model (even though it\'s not yet valid)');
        return true;
      }
      return false;
    }
    // Geometry and Precomputation not yet supported.
    if (!(config.decompositionCategory == 'parameter' ||
          config.decompositionCategory == 'qoi')) {
      return false;
    }
    if (!config.decompositionField) {
      return false;
    }
    return true;
  }

  /**
   * Fetch the morse smale complex persistence for the given options.
   */
  async fetchDecomposition() {
    let k = 15; // num nearest neighbors to consider when generating M-S complex for a dataset
    let datasetId = this.state.datasetId;
    let category = this.state.decompositionCategory;
    let field = this.state.decompositionField;
    let result = await this.client.fetchMorseSmalePersistence(datasetId, category, field, k);
    this.setState({
      minPersistence: result.minPersistenceLevel,
      maxPersistence: result.maxPersistenceLevel,
      complexSizes: result.complexSizes,
      sliderPersistence: result.maxPersistenceLevel,
      persistenceLevel: ('' + result.maxPersistenceLevel),
    });
    this.updateDataModel('' + result.maxPersistenceLevel);
  }

  componentDidMount() {
    if (this.state.decompositionMode == 'Morse-Smale') {
      let k = 15; // num nearest neighbors to consider when generating M-S complex for a dataset
      let datasetId = this.state.datasetId;
      let category = this.state.decompositionCategory;
      let field = this.state.decompositionField;
      this.client.fetchMorseSmalePersistence(datasetId, category, field, k)
        .then(function(result) {
          this.setState({
            minPersistence: result.minPersistenceLevel,
            maxPersistence: result.maxPersistenceLevel,
            complexSizes: result.complexSizes,
            sliderPersistence: result.maxPersistenceLevel,
            persistenceLevel: ('' + result.maxPersistenceLevel),
          });
          this.updateDataModel('' + result.maxPersistenceLevel);
        }.bind(this));
    } else {
      this.setState({
        minPersistence: null,
        maxPersistence: null,
        crystals: [],
      });
    }
  }

  /**
   * Handle change of properties and state.
   * @param {object} prevProps
   * @param {object} prevState
   * @param {object} snapshot
   */
  componentDidUpdate(prevProps, prevState, snapshot) {
    if (prevState.decompositionMode !== this.state.decompositionMode ||
        prevState.decompositionCategory !== this.state.decompositionCategory ||
        prevState.decompositionField !== this.state.decompositionField) {
      let config = {
        decompositionMode: this.state.decompositionMode,
        decompositionCategory: this.state.decompositionCategory,
        decompositionField: this.state.decompositionField,
      };
      if (this.decompositionConfigValid(config)) {
        console.log('ready to fetch decomposition');
        this.fetchDecomposition();
      }
    }
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
      let k = 15; // num nearest neighbors to consider when generating M-S complex for a dataset
      let datasetId = this.state.datasetId;
      let category = this.state.decompositionCategory;
      let field = this.state.decompositionField;
      this.client.fetchMorseSmalePersistence(datasetId, category, field, k)
        .then(function(result) {
          this.setState({
            minPersistence: result.minPersistenceLevel,
            maxPersistence: result.maxPersistenceLevel,
            complexSizes: result.complexSizes,
            sliderPersistence: result.maxPersistenceLevel,
            persistenceLevel: ('' + result.maxPersistenceLevel),
          });
          this.updateDataModel('' + result.maxPersistenceLevel);
        }.bind(this));
    } else {
      this.setState({
        minPersistence: null,
        maxPersistence: null,
        crystals: [],
      });
      if (this.props.onDatasetChange) {
        this.props.onDatasetChange(null);
      }
    }
  }

  handleNearestNeighborChange(event) {
    let neighborhoodSize = event.target.value;
    this.setState((prevState) => ({
      ms: { ...prevState.ms, knn:neighborhoodSize },
    }));
  }

  handleSigmaChange(event) {
    let sigma = event.target.value;
    this.setState((prevState) => ({
      ms: { ...prevState.ms, sigma:sigma },
    }));
  }

  handleSmoothingChange(event) {
    let smooth = event.target.value;
    this.setState((prevState) => ({
      ms: { ...prevState.ms, smoothing:smooth },
    }));
  }

  handleRecomputeMorseSmale() {
    console.log('Computing the ms object');
    console.log(this.state.ms);
  }

  handleExportMorseSmale() {
    console.log('Dumping the ms object');
    console.log(this.state.ms);
  }

  handleModelSigmaChange(event) {
    let sigma = event.target.value;
    this.setState((prevState) => ({
      model: { ...prevState.model, sigma:sigma },
    }));
  }

  /**
   * Handle the decomposition field category changing.
   * @param {object} event
   */
  handleDecompositionCategoryChange(event) {
    let category = event.target.value;
    this.setState({
      decompositionCategory: category,
    });
  }

  /**
   * Handle the decomposition field changing.
   * @param {object} event
   */
  handleDecompositionFieldChange(event) {
    let field = event.target.value;
    this.setState({
      decompositionField: field,
    });
  }

  /**
   * Updates state when active persistence level changes.
   * @param {string} level
   */
  async updateDataModel(level) {
    if (level != '') {
      let k = 15; // num nearest neighbors to consider when generating M-S complex for a dataset
      let datasetId = this.state.datasetId;
      let persistenceLevel = parseInt(level);
      let category = this.state.decompositionCategory;
      let field = this.state.decompositionField;
      let result = await this.client.fetchMorseSmalePersistenceLevel(datasetId, category, field, k, persistenceLevel);

      this.setState({
        crystals: result.complex.crystals,
      });

      this.props.onDecompositionChange({
        datasetId: result.datasetId,
        decompositionMode: this.state.decompositionMode,
        decompositionCategory: this.state.decompositionCategory,
        decompositionField: this.state.decompositionField,
        k: result.k,
        persistenceLevel: result.persistenceLevel,
      });
    } else {
      this.setState({
        crystals: [],
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
      sliderPersistence: level,
    });

    this.updateDataModel(level);
  }

  /**
   * Handles when the persistence slider is changed.
   * @param {Event} event
   */
  handleSliderChange(event) {
    let value = event.target.value;
    this.setState({
      sliderPersistence: value,
    });
  }

  /**
   * Handles when the user releases control of the persistence slider.
   * @param {Event} event
   */
  handleSliderRelease(event) {
    if (this.state.sliderPersistence != this.state.persistenceLevel) {
      this.setState({
        persistenceLevel: '' + this.state.sliderPersistence,
      });
      this.updateDataModel('' + this.state.sliderPersistence);
    }
  }

  /**
   * Handles when a user toggles a crystal visibility on or off.
   * @param {number} index - Which crystal is being toggled.
   */
  handleCrystalToggle(index) {
    let crystals = this.state.crystals;
    let isDisabled = crystals[index].isDisabled;
    crystals[index].isDisabled = !isDisabled;
    this.setState({ crystals:crystals });
  }

  /**
   * A partial render of decomposition field UI.
   * @return {JSX}
   */
  _getDecompositionFieldMenuItems() {
    let items = [];
    switch (this.state.decompositionCategory) {
      case 'parameter':
        items = this.props.dataset.parameterNames.map((name) => (
          <MenuItem value={name} key={name}>
            {name}
          </MenuItem>
        ));
        break;
      case 'qoi':
        items = this.props.dataset.qoiNames.map((name) => (
          <MenuItem value={name} key={name}>
            {name}
          </MenuItem>
        ));
        break;
      default:
        break;
    };

    return items;
  }

  /**
   * Renders the component to HTML.
   * @return {HTML}
   */
  render() {
    const { classes } = this.props;
    let persistenceLevels = [];
    if (this.state.minPersistence != null
        && this.state.maxPersistence != null) {
      for (let i=this.state.maxPersistence;
        i >= this.state.minPersistence; i--) {
        persistenceLevels.push(i);
      }
    }
    return (
      // TODO: set disabled only when there's no case data.
      <ExpansionPanel disabled={!this.props.enabled || !this.props.dataset}
        defaultExpanded={true} style={{ paddingLeft:'0px', margin:'1px' }}>
        <ExpansionPanelSummary expandIcon={ <ExpandMoreIcon/> }>
          <Typography>Decomposition</Typography>
        </ExpansionPanelSummary>
        <ExpansionPanelDetails style={{ paddingLeft: '15px',
          paddingRight: '10px', margin: '1px', width: '100%',
          boxSizing: 'border-box' }}>
          <div style={{ display: 'flex', flexDirection: 'column',
            width: '100%', boxSizing: 'border-box' }}>

            { /* Field Category (Decomposition) Dropdown */ }
            <FormControl className={classes.formControl}
              disabled={!this.props.enabled || !this.props.dataset}>
              <InputLabel htmlFor='category-input'>Field Category</InputLabel>
              <Select ref="categoryCombo"
                value={this.state.decompositionCategory || ''}
                onChange={this.handleDecompositionCategoryChange} inputProps={{
                  name: 'category',
                  id: 'category-input',
                }}>
                <MenuItem value="">
                  <em>None</em>
                </MenuItem>
                <MenuItem value="parameter">
                  <em>Parameter</em>
                </MenuItem>
                <MenuItem value="geometry" disabled={true}>
                  <em>Geometry</em>
                </MenuItem>
                <MenuItem value="qoi">
                  <em>QoI</em>
                </MenuItem>
                <MenuItem value="precomputed" disabled={true}>
                  <em>Precomputed</em>
                </MenuItem>
              </Select>
            </FormControl>

            { /* Field Dropdown */ }
            <FormControl className={classes.formControl}
              disabled={ !this.props.enabled || !this.props.dataset
                || !this.state.decompositionCategory}>
              <InputLabel htmlFor='field-input'>Field</InputLabel>
              <Select ref="fieldCombo"
                value={this.state.decompositionField || ''}
                onChange={this.handleDecompositionFieldChange} inputProps={{
                  name: 'field',
                  id: 'field-input',
                }}>
                <MenuItem value="">
                  <em>None</em>
                </MenuItem>
                {
                  this._getDecompositionFieldMenuItems()
                }
              </Select>
            </FormControl>

            {/* Partitioning Algorithm */ }
            <FormControl className={classes.formControl}
              disabled={!this.props.enabled}
              style={{ width: '100%',
                boxSizing: 'border-box',
                paddingRight: '10px' }}>
              <InputLabel htmlFor='mode-field'>Partitioning Algorithm</InputLabel>
              <Select ref="decompositionCombo"
                disabled={!this.props.enabled || !this.props.dataset}
                value={this.state.decompositionMode}
                style={{ width:'100%' }}
                onChange={this.handleDecompositionModeChange}
                inputProps={{
                  name: 'mode',
                  id: 'mode-field',
                }}>
                <MenuItem value='Morse-Smale'>
                  <em>Morse-Smale</em>
                </MenuItem>
              </Select>
            </FormControl>

            {/* K Nearest Neighbors */}
            <TextField
              label="Neighborhood Size"
              id="ms-knn"
              defaultValue={this.state.ms.knn}
              size="small"
              type="number"
              onChange={this.handleNearestNeighborChange}
            />

            {/* Sigma */}
            <TextField
              label="Sigma"
              id="ms-sigma"
              defaultValue={this.state.ms.sigma}
              size="small"
              onChange={this.handleSigmaChange.bind(this)}
            />

            {/* Smooth */}
            <TextField
              label="Smoothing"
              id="ms-smooth"
              defaultValue={this.state.ms.smoothing}
              size="small"
              onChange={this.handleSmoothingChange.bind(this)}
            />

            { /* Button to dump crystal partitions to disk */}
            { this.state.devMode && <Button size="small" onClick={this.handleRecomputeMorseSmale.bind(this)}>Recompute Morse-Smale</Button> }
            { this.state.devMode && <Button size="small" onClick={this.handleExportMorseSmale.bind(this)}>Export Crystal Partitions</Button> }

            { /* Interpolation Model Selection */}
            <FormControl className={classes.formControl}
              disabled={!this.props.enabled}
              style={{ width: '100%',
                boxSizing: 'border-box',
                paddingRight: '10px' }}>
              <InputLabel htmlFor='model-field'>Interpolation Model</InputLabel>
              <Select ref="interpolationCombo"
                disabled={!this.props.enabled || !this.props.dataset}
                value={this.state.interpolationModel || ''}
                style={{ width:'100%' }}
                onChange={this.handleInterpolationModelChange} 
                inputProps={{
                  name: 'model',
                  id: 'model-field',
                }}>
                <MenuItem value='None'>
                  <em>None</em>
                </MenuItem>
                <MenuItem value='pca'>
                  <em>PCA</em>
                </MenuItem>
                <MenuItem value='shapeodds'>
                  <em>ShapeOdds</em>
                </MenuItem>
                <MenuItem value='sharedgp' disabled={true}>
                  <em>Shared-GP</em>
                </MenuItem>
              </Select>
            </FormControl>

            { /* Interpolation Model [Gaussian] sigma bandwidth parameter */ }
            <TextField
              label="sigma bandwidth percent"
              id="model-sigma"
              defaultValue={this.state.model.sigma}
              size="small"
              onChange={this.handleModelSigmaChange.bind(this)}
            />

            { /* Histogram of data partitions (each consisting of n subpartitions) */ }
            <div style={{ height:'15px' }}></div>
            {
              persistenceLevels.length > 0 ? [
                <Histogram key="histogram" size={[190, 100]}
                  data={this.state.complexSizes} brushEnabled={false}/>,
                <input key="slider" type="range" step={1} id="myRange"
                  min={this.state.minPersistence}
                  max={this.state.maxPersistence}
                  value={this.state.sliderPersistence}
                  onChange={this.handleSliderChange}
                  onMouseUp={this.handleSliderRelease}
                  style={{ width: '190px', height: '15px', borderRadius: '5px',
                    background: '#d3d3d3', outline: 'none', opacity: '0.7',
                    transition: 'opacity .2s', paddingLeft: '0px',
                    marginLeft: '0px' }} />,
                <FormControl key="persistenceCombo"
                  disabled={!this.props.enabled}
                  className={classes.formControl}
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
                </FormControl>,
              ] : []
            }

            <div style={{ height:'5px' }}></div>
            <List style={{ maxHeight:'200px', overflow:'auto' }}>
              {
                this.state.crystals.map((crystal, i) => (
                  <ListItem key={i} style={{ padding:'1px' }}
                    dense button
                    onClick={() => this.handleCrystalToggle(i)}>
                    <Checkbox checked={!this.state.crystals[i].isDisabled}
                      tabIndex={-1} disableRipple
                      color="primary" style={{ paddingLeft:0 }}/>
                    <ListItemText primary={'Crystal ' + (i+1)} />
                  </ListItem>
                ))
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
export default withDSXContext(withStyles({})(DecompositionPanel));
