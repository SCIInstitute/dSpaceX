import Checkbox from '@material-ui/core/Checkbox';
import ExpandMoreIcon from '@material-ui/icons/ExpandMore';
import ExpansionPanel from '@material-ui/core/ExpansionPanel';
import ExpansionPanelDetails from '@material-ui/core/ExpansionPanelDetails';
import ExpansionPanelSummary from '@material-ui/core/ExpansionPanelSummary';
import FormControl from '@material-ui/core/FormControl';
import FormControlLabel from '@material-ui/core/FormControlLabel';
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

    this.handleDecompositionModeChange = this.handleDecompositionModeChange.bind(this);
    this.handleDecompositionCategoryChange = this.handleDecompositionCategoryChange.bind(this);
    this.handleDecompositionFieldChange = this.handleDecompositionFieldChange.bind(this);
    this.handlePersistenceLevelChange = this.handlePersistenceLevelChange.bind(this);
    this.handlePersistenceSliderChange = this.handlePersistenceSliderChange.bind(this);
    this.handlePersistenceSliderRelease = this.handlePersistenceSliderRelease.bind(this);
    this.handleCrystalToggle = this.handleCrystalToggle.bind(this);
    this._getDecompositionFieldMenuItems = this._getDecompositionFieldMenuItems.bind(this);
    this.decompositionConfigValid = this.decompositionConfigValid.bind(this);
    this.fetchDecomposition = this.fetchDecomposition.bind(this);
    this.clearDecompositionState = this.clearDecompositionState.bind(this);

    this.state = {
      devMode: false,

      datasetId: this.props.dataset.datasetId,

      interpolationModel: 'pca',  /*hardcoded for darpa demo -> TODO: use actual models that are read*/
      model: {
        sigma: 0.15,
      },

      decompositionMode: 'Morse-Smale',
      decompositionCategory: 'qoi',
      decompositionField: this.props.dataset.qoiNames[0],

      ms: {
        knn: 15,
        sigma: 0.25,
        smooth: 15.0,
        curvepoints: 55,
        depth: -1,
        noise: true,
        normalize: true,
      },

      // decomposition state
      persistenceLevel: -1,
      minPersistence: null,
      maxPersistence: null,
      complexSizes: [],
      crystals: [],
      sliderPersistence: null,
    };

    this.client = this.props.dsxContext.client;
  }

  /**
   * Clear decomposition state
   */
  clearDecompositionState() {
    this.setState({
      persistenceLevel: -1,
      minPersistence: null,
      maxPersistence: null,
      complexSizes: [],
      crystals: [],
      sliderPersistence: null,
    });
  }

  /**
   * Returns true if the current decomposoition configuration state is valid.
   * @return {boolean}
   */
  decompositionConfigValid() {
    const { decompositionMode, decompositionCategory, decompositionField } = this.state;

    // Verify category (geometry and precomputed not yet supported)
    if (!(decompositionCategory == 'parameter' ||
          decompositionCategory == 'qoi')) {
      console.log('decompositionPanel current configuration is invalid:\n\tcategory '+decompositionCategory+' not yet supported');
      return false;
    }

    // Ensure a field is selected
    if (!decompositionField) {
      console.log('decompositionPanel current configuration is invalid:\n\tno field selected');
      return false;
    }

    // validate current decomposition mode and its parameters
    if (decompositionMode === 'Morse-Smale') {
      const { knn, sigma, smooth, noise, depth, curvepoints, normalize } = this.state.ms;
      if (knn < 0) {
        console.log('current M-S configuration is invalid:\n\tknn must be >= 0');
        return false;
      }
      if (sigma < 0.0) {
        console.log('current M-S configuration is invalid:\n\tsigma must be >= 0');
        return false;
      }
      if (smooth < 0.0) {
        console.log('current M-S configuration is invalid:\n\tsmooth must be >= 0');
        return false;
      }
      if (depth <= 0.0 && depth != -1) {
        console.log('current M-S configuration is invalid:\n\must compute at least one (depth > 0) or all (-1) persistence levels');
        return false;
      }
      if (curvepoints < 3) {
        console.log('current M-S configuration is invalid:\n\curvepoints must be >= 3');
        return false;
      }
    }
    else {
      console.log('decompositionPanel current configuration is invalid:\n\tunknown mode: '+decompositionMode);
      return false;
    }

    return true;
  }

  /**
   * Fetch the morse smale complex persistence for the given options.
   */
  async fetchDecomposition() {
    if (this.decompositionConfigValid())
    {
      if (this.state.decompositionMode == 'Morse-Smale') {
        let datasetId = this.state.datasetId;
        let category = this.state.decompositionCategory;
        let field = this.state.decompositionField;
        const { knn, sigma, smooth, noise, depth, curvepoints, normalize } = this.state.ms;
        await this.client.fetchMorseSmaleDecomposition(datasetId, category, field, knn, sigma, smooth, noise, depth, curvepoints, normalize)
          .then(function(result) {
            if (!result.error) {
              this.setState({
                minPersistence: result.minPersistenceLevel,
                maxPersistence: result.maxPersistenceLevel,
                complexSizes: result.complexSizes,
                sliderPersistence: result.maxPersistenceLevel,
                persistenceLevel: result.maxPersistenceLevel});
              this.updateDataModel();
            }
            else {
              console.log('decompositionPanel.fetchDecomposition: fetch decomposition from server failed:\n\t'+result.error_msg);
            }
          }.bind(this));
      }
      else {
        console.log('decompositionPanel.fetchDecomposition: \n\tunknown decomposition mode: '+this.state.decompositionMode);
        this.clearDecompositionState();
      }
    }
    else {
      console.log('decompositionPanel.fetchDecomposition: \n\tinvalid decomposition parameters');
      this.clearDecompositionState();
    }
  }
  
  componentDidMount() {
    this.fetchDecomposition();
  }

  /**
   * Handle change of properties and state.
   * @param {object} prevProps
   * @param {object} prevState
   * @param {object} snapshot
   */
  componentDidUpdate(prevProps, prevState, snapshot) {
    // overview: - when component state updates, fetch the new decomposition only if the mode, category, or field changes
    //             todo <ctc> (category should trigger a field change, so really doesn't need to be checked explicitly)
    //           - if ms params change, don't recompute unless user clicks the button to do so

    if (prevState.decompositionMode !== this.state.decompositionMode ||
        //prevState.decompositionCategory !== this.state.decompositionCategory ||
        prevState.decompositionField !== this.state.decompositionField) {
      console.log('decompositionPanel.componentDidUpdate:\n\tdecomposition state (i.e., field) changed, fetching new decomposition...');
      this.fetchDecomposition();
    }
    else if (prevState.persistenceLevel !== this.state.persistenceLevel) {
      console.log('Persistence level changed, updating data model...');
      this.updateDataModel();
    }
    else {
      console.log('decompositionPanel.componentDidUpdate, but state has not changed.');
    }
  }

  /**
   * Passes config changes up the line.
   */
  updatePropsConfig() {
    this.props.onDecompositionChange({
      datasetId: this.state.datasetId,
      decompositionMode: this.state.decompositionMode,
      decompositionCategory: this.state.decompositionCategory,
      decompositionField: this.state.decompositionField,
      k: this.state.ms.knn,
      persistenceLevel: this.state.persistenceLevel,
    });
  }
  
  // <ctc> todo: convert the following handle*Change to inline lambdas
  
  /**
   * Handles when the decomposition combo is changed.
   * @param {Event} event
   */
  handleDecompositionModeChange(event) {
    let mode = event.target.value;
    this.setState({
      decompositionMode: mode,
    });

    // <ctc> this should already be called by componentDidUpdate
    //this.fetchDecomposition();
    console.log('decompositionPanel.handleDecompositionModeChange (should updated automatically)');
  }

  handleMSknnChange(event) {
    let neighborhoodSize = event.target.value;
    this.setState((prevState) => ({
      ms: { ...prevState.ms, knn:neighborhoodSize },
    }));
  }

  handleMSSigmaChange(event) {
    let sigma = parseFloat(event.target.value);
    this.setState((prevState) => ({
      ms: { ...prevState.ms, sigma:sigma },
    }));
  }

  handleMSSmoothChange(event) {
    let smooth = event.target.value;
    this.setState((prevState) => ({
      ms: { ...prevState.ms, smooth:smooth },
    }));
  }

  handleMSCurvePointsChange(event) {
    let curvepoints = event.target.value;
    this.setState((prevState) => ({
      ms: { ...prevState.ms, curvepoints:curvepoints },
    }));
  }

  handleMSDepthChange(event) {
    let persistenceDepth = event.target.value;
    this.setState((prevState) => ({
      ms: { ...prevState.ms, depth:persistenceDepth },
    }));
  }

  handleMSNoiseChange(event) {
    let addNoise = event.target.checked;
    this.setState((prevState) => ({
      ms: { ...prevState.ms, noise:addNoise },
    }));
  }

  handleMSNormalizeChange(event) {
    let normalize = event.target.checked;
    this.setState((prevState) => ({
      ms: { ...prevState.ms, normalize:normalize },
    }));
  }

  handleRecomputeMorseSmale() {
    console.log('recomputing the ms object...');
    this.fetchDecomposition();
  }

  handleExportMorseSmale() {
    console.log('Dumping the ms object (TODO)');
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

    // <ctc> see! This one updates the field list automatically, right? But maybe that's in render...
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

    // <ctc> But this one *must* call componentDidUpdate
  }

  /**
   * Updates state when active persistence level changes.
   * @param {string} level
   */
  async updateDataModel(level) {
    let persistenceLevel = this.state.persistenceLevel;
    if (persistenceLevel >= this.minPersistence && persistenceLevel <= this.maxPersistence) {
      if (this.state.decompositionMode == 'Morse-Smale') {
        // annoying (and error prone) to have to send all the same parameters to this function as to fetchDecomposition (fixme)
        let datasetId = this.state.datasetId;
        let category = this.state.decompositionCategory;
        let field = this.state.decompositionField;
        const { knn, sigma, smooth, noise, depth, curvepoints, normalize } = this.state.ms;
        await this.client.fetchMorseSmalePersistenceLevel(datasetId, category, field, persistenceLevel, knn, sigma, smooth, noise, depth, curvepoints, normalize)
          .then(function(result) {
            if (!result.error) {
              this.setState({
                crystals: result.complex.crystals,
              });
              this.updatePropsConfig();
            }
            else {
              this.setState({ crystals: [] });
              console.log('decompositionPanel.updateDataModel failed: \n\t'+result.error_msg);
            }
          });
      }
      else {
        console.log('decompositionPanel.updateDataModel: \n\tunknown decomposition mode');
      }
    }
    else {
      console.log('decompositionPanel.updateDataModel: \n\tpersistenceLevel ('+persistenceLevel+') out of range ('+this.state.minPersistence+', '+this.state.maxPersistence+')');
      this.setState({ crystals: [] });
    }
  }

  /**
   * Handles when the persistence level combo is changed.
   * @param {Event} event
   */
  handlePersistenceLevelChange(event) {
    let level = parseInt(event.target.value);
    this.setState({
      persistenceLevel: level,
      sliderPersistence: level,
    });

    // <ctc> this should already be called by componentDidUpdate
    //this.updateDataModel(level);
    console.log('decompositionPanel.handlePersistenceLevelChange (should updated automatically)');
  }

  /**
   * Handles when the persistence slider is changed.
   * @param {Event} event
   */
  handlePersistenceSliderChange(event) {
    let value = event.target.value;
    this.setState({
      sliderPersistence: value,
    });
    // <ctc> todo: verify this returns an int and not a string ... not sure this callback is even necessary w/ handleSliderRelease
  }

  /**
   * Handles when the user releases control of the persistence slider.
   * @param {Event} event
   */
  handlePersistenceSliderRelease(event) {
    if (this.state.sliderPersistence != this.state.persistenceLevel) {
      this.setState({
        persistenceLevel: this.state.sliderPersistence,
      });

      //this.updateDataModel(this.state.sliderPersistence);
      console.log('decompositionPanel.handlePersistenceLevelChange (should updated automatically)');
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
              onChange={this.handleMSknnChange}
            />

            {/* Sigma */}
            <TextField
              label="Smooth Curves"
              id="ms-sigma"
              defaultValue={this.state.ms.sigma}
              size="small"
              onChange={this.handleMSSigmaChange.bind(this)}
            />

            {/* Smooth */}
            <TextField
              label="Smooth Topology"
              id="ms-smooth"
              defaultValue={this.state.ms.smooth}
              size="small"
              onChange={this.handleMSSmoothChange.bind(this)}
            />

            {/* Depth */}
            <TextField
              label="Persistence Depth"
              id="ms-depth"
              defaultValue={this.state.ms.depth}
              size="small"
              type="number"
              onChange={this.handleMSDepthChange.bind(this)}
            />

            {/* Add noise */}
            <FormControlLabel
              control={<Checkbox checked={this.state.ms.noise} 
                                 onChange={this.handleMSNoiseChange}
                                 name="msNoiseCheckbox" />}
              label="Add noise"
            />

            {/* Scale normalize */}
            <FormControlLabel
              control={<Checkbox checked={this.state.ms.normalize} 
                                 onChange={this.handleMSNormalizeChange}
                                 name="msNormalizeCheckbox" />}
              label="Scale normalize field"
            />

            {/* Curve points */}
            <TextField
              label="Crystal curve points"
              id="ms-curvepoints"
              defaultValue={this.state.ms.curvepoints}
              size="small"
              onChange={this.handleMSCurvePointsChange.bind(this)}
            />

            { /* Buttons to recompute M-S and dump crystal partitions to disk */}
            { <Button size="small" onClick={this.handleRecomputeMorseSmale.bind(this)}>Recompute Morse-Smale</Button> }
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
                  onChange={this.handlePersistenceSliderChange}
                  onMouseUp={this.handlePersistenceSliderRelease}
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
