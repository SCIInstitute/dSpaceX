import { Accordion, AccordionDetails, AccordionSummary } from '@material-ui/core';
import { Button } from '@material-ui/core';
import Checkbox from '@material-ui/core/Checkbox';
import ExpandMoreIcon from '@material-ui/icons/ExpandMore';
import FormControl from '@material-ui/core/FormControl';
import FormControlLabel from '@material-ui/core/FormControlLabel';
import FormGroup from '@material-ui/core/FormGroup';
import FormHelperText from '@material-ui/core/FormHelperText';
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

    this._getDecompositionFieldMenuItems = this._getDecompositionFieldMenuItems.bind(this);

    this.state = {
      devMode: true,

      datasetId: this.props.dataset.datasetId,
      /* hardcoded for darpa demo -> TODO: use actual models that are read just like decompositionField */
      interpolationModel: 'pca',
      model: {
        sigma: 0.15,
      },

      decompositionMode: 'Morse-Smale',
      decompositionCategory: 'qoi',
      decompositionField: this.props.dataset.qoiNames ? this.props.dataset.qoiNames[0] : '',

      ms: {
        knn: 15,
        sigma: 0.25,
        smooth: 15.0,
        curvepoints: 50,
        depth: 20,
        noise: true,
        normalize: true,
      },

      // decomposition state
      persistenceLevel: -1,
      minPersistence: -1,
      maxPersistence: -1,
      complexSizes: [],
      crystals: [],
      sliderPersistence: null, // just some bogus thing to keep around to handle slider and pulldown updates
    };

    this.client = this.props.dsxContext.client;
  }

  /**
   * Clear decomposition state
   */
  clearDecompositionState() {
    this.setState({
      persistenceLevel: -1,
      minPersistence: -1,
      maxPersistence: -1,
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
      console.log('decompositionPanel current configuration is invalid:' +
          '\n\tcategory '+decompositionCategory+' not yet supported');
      return false;
    }

    // Ensure a field is selected
    if (!decompositionField) {
      console.log('decompositionPanel current configuration is invalid:\n\tno field selected');
      return false;
    }

    // validate current decomposition mode and its parameters
    if (decompositionMode === 'Morse-Smale') {
      const { knn, sigma, smooth, depth, curvepoints } = this.state.ms;
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
      if (depth <= 0 && depth != -1) {
        console.log('current M-S configuration is invalid:' +
            '\n\must compute at least one (depth > 0) or all (-1) persistence levels');
        return false;
      }
      if (curvepoints < 3) {
        console.log('current M-S configuration is invalid:\n\curvepoints must be >= 3');
        return false;
      }
    } else {
      console.log('decompositionPanel current configuration is invalid:\n\tunknown mode: '+decompositionMode);
      return false;
    }

    return true;
  }

  /**
   * Fetch the morse smale complex persistence for the given options.
   */
  async fetchDecomposition() {
    if (this.decompositionConfigValid()) {
      if (this.state.decompositionMode == 'Morse-Smale') {
        let datasetId = this.state.datasetId;
        let category = this.state.decompositionCategory;
        let field = this.state.decompositionField;
        const { knn, sigma, smooth, noise, depth, curvepoints, normalize } = this.state.ms;
        console.log('decompositionPanel.fetchDecomposition: fetching decomposition for '+field+' from server...\n');
        await this.client.fetchMorseSmaleDecomposition(datasetId,
          category, field, knn, sigma, smooth, noise, depth, curvepoints, normalize)
          .then(function(result) {
            if (!result.error) {
              // console.log('decompositionPanel.fetchDecomposition succeeded: setting state (mp:'
              // +result.minPersistenceLevel+',Mp:'+result.maxPersistenceLevel+',cs:'+result.complexSizes+')\n');
              this.setState({
                minPersistence: result.minPersistenceLevel,
                maxPersistence: result.maxPersistenceLevel,
                complexSizes: result.complexSizes,
                sliderPersistence: result.maxPersistenceLevel,
                persistenceLevel: result.maxPersistenceLevel });
              // not calling updateDataModel since componentDidUpdate calls it
            } else {
              console.log('decompositionPanel.fetchDecomposition: fetch decomposition from server failed:\n\t'
                  + result.error_msg);
            }
          }.bind(this));
      } else {
        console.log('decompositionPanel.fetchDecomposition: \n\tunknown decomposition mode: '
            + this.state.decompositionMode);
        this.clearDecompositionState();
      }
    } else {
      console.log('decompositionPanel.fetchDecomposition: \n\tinvalid decomposition parameters');
      this.clearDecompositionState();
    }
  }

  /**
   * React lifecycle method called when componenet mounts
   */
  componentDidMount() {
    // console.log('decompositionPanel component mounted: fetching decomposition');
    this.fetchDecomposition();
  }

  /**
   * Handle change of properties and state.
   * @param {object} prevProps
   * @param {object} prevState
   * @param {object} snapshot
   */
  componentDidUpdate(prevProps, prevState, snapshot) {
    // overview: - when component state updates, fetch the new decomposition only if the mode or field changes
    //           - category triggers a field change, so doesn't need to be checked here
    //           - if ms params change, don't recompute unless user clicks the button to do so

    if (prevState.decompositionMode !== this.state.decompositionMode ||
        prevState.decompositionField !== this.state.decompositionField) {
      // console.log('decompositionPanel.componentDidUpdate: field changed, fetching new decomposition...');
      this.fetchDecomposition();
    } else if (prevState.persistenceLevel !== this.state.persistenceLevel) {
      console.log('Persistence level changed from '
          + prevState.persistenceLevel +' to '+this.state.persistenceLevel+', updating data model...');
      this.updateDataModel();
    } else {
      // console.log('decompositionPanel.componentDidUpdate, but state has not changed.');
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

  handleMSknnChange(event) {
    let neighborhoodSize = parseInt(event.target.value);
    if (!isNaN(neighborhoodSize)) {
      this.setState((prevState) => ({
        ms: { ...prevState.ms, knn:neighborhoodSize },
      }));
    }
  }

  handleMSSigmaChange(event) {
    let sigma = parseFloat(event.target.value);
    if (!isNaN(sigma)) {
      this.setState((prevState) => ({
        ms: { ...prevState.ms, sigma:sigma },
      }));
    }
  }

  handleMSSmoothChange(event) {
    let smooth = parseFloat(event.target.value);
    if (!isNaN(smooth)) {
      this.setState((prevState) => ({
        ms: { ...prevState.ms, smooth:smooth },
      }));
    }
  }

  handleMSCurvePointsChange(event) {
    let curvepoints = parseInt(event.target.value);
    if (!isNaN(curvepoints)) {
      this.setState((prevState) => ({
        ms: { ...prevState.ms, curvepoints:curvepoints },
      }));
    }
  }

  handleMSDepthChange(event) {
    let persistenceDepth = parseInt(event.target.value);
    if (!isNaN(persistenceDepth)) {
      this.setState((prevState) => ({
        ms: { ...prevState.ms, depth:persistenceDepth },
      }));
    }
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
    this.fetchDecomposition();
  }

  handleExportMorseSmale() {
    console.log('Dumping the ms object (TODO)...');
  }

  handleModelSigmaChange(event) {
    let sigma = event.target.value;
    if (!isNaN(sigma)) {
      this.setState((prevState) => ({
        model: { ...prevState.model, sigma:sigma },
      }));
    }
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
   * Handle the interpolation model changing.
   * @param {object} event
   */
  handleInterpolationModelChange(event) {
    let model = event.target.value;
    this.setState({
      interpolationModel: model,
    });
  }

  /**
   * Updates state when active persistence level changes.
   * @param {string} level
   */
  async updateDataModel(level) {
    let persistenceLevel = this.state.persistenceLevel;
    if (persistenceLevel >= this.state.minPersistence && persistenceLevel <= this.state.maxPersistence) {
      if (this.state.decompositionMode == 'Morse-Smale') {
        // annoying (and error prone) to have to send all the
        // same parameters to this function as to fetchDecomposition (fixme)
        let datasetId = this.state.datasetId;
        let category = this.state.decompositionCategory;
        let field = this.state.decompositionField;
        const { knn, sigma, smooth, noise, depth, curvepoints, normalize } = this.state.ms;
        console.log('decompositionPanel.updateDataModel: fetching persistence level '
            + persistenceLevel + ' of decomposition...\n');
        await this.client.fetchMorseSmalePersistenceLevel(datasetId,
          category, field, persistenceLevel, knn, sigma, smooth, noise, depth, curvepoints, normalize)
          .then(function(result) {
            if (!result.error) {
              this.setState({
                crystals: result.complex.crystals,
              });
              this.updatePropsConfig();
              // console.log('decompositionPanel.updateDataModel succeeded. Props config updated.');
            } else {
              this.setState({ crystals:[]});
              console.log('decompositionPanel.updateDataModel failed: \n\t'+result.error_msg);
            }
          }.bind(this));
      } else {
        console.log('decompositionPanel.updateDataModel failed: \n\tunknown decomposition mode');
        this.setState({ crystals:[]});
      }
    } else {
      console.log('decompositionPanel.updateDataModel failed: \n\tpersistenceLevel ('
          + persistenceLevel + ') out of range (' + this.state.minPersistence+', ' + this.state.maxPersistence + ')');
      this.setState({ crystals:[]});
    }
  }

  /**
   * Handles when the persistence level combo is changed.
   * @param {Event} event
   */
  handlePersistenceLevelChange(event) {
    let level = event.target.value;
    this.setState({
      persistenceLevel: parseInt(level),
      sliderPersistence: level,
    });
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
  }

  /**
   * Handles when the user releases control of the persistence slider.
   * @param {Event} event
   */
  handlePersistenceSliderRelease(event) {
    if (this.state.sliderPersistence != this.state.persistenceLevel) {
      this.setState({
        persistenceLevel: parseInt(this.state.sliderPersistence),
      });
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
    if (this.state.minPersistence >= 0 && this.state.maxPersistence >= this.state.minPersistence) {
      for (let i=this.state.maxPersistence; i >= this.state.minPersistence; i--) {
        persistenceLevels.push(i);
      }
    }
    return (
      // TODO: set disabled only when there's no case data.
      <Accordion disabled={!this.props.enabled || !this.props.dataset}
        defaultExpanded={true} style={{ paddingLeft:'0px', margin:'1px' }}>
        <AccordionSummary expandIcon={ <ExpandMoreIcon/> }>
          <Typography>Decomposition</Typography>
        </AccordionSummary>
        <AccordionDetails style={{ paddingLeft: '15px',
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
                onChange={this.handleDecompositionCategoryChange.bind(this)} inputProps={{
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
                onChange={this.handleDecompositionFieldChange.bind(this)} inputProps={{
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
            <Accordion disabled={!this.props.enabled} defaultExpanded={false}
              style={{ paddingLeft:'0px', margin:'1px' }}>
              <AccordionSummary expandIcon={ <ExpandMoreIcon/> }>
                <Typography>Partitioning</Typography>
              </AccordionSummary>
              <AccordionDetails style={{ paddingLeft: '0px',
                paddingRight: '10px', margin: '1px', width: '100%',
                boxSizing: 'border-box' }}>
                <div style={{ display: 'flex', flexDirection: 'column',
                  width: '100%', boxSizing: 'border-box' }}>

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
                      onChange={this.handleDecompositionModeChange.bind(this)}
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
                    InputProps={{ inputProps:{ min:1 } }}
                    onChange={this.handleMSknnChange.bind(this)}
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
                    InputProps={{ inputProps:{ min:-1 } }}
                    onChange={this.handleMSDepthChange.bind(this)}
                  />

                  {/* Curve points */}
                  <TextField
                    label="Crystal curve points"
                    id="ms-curvepoints"
                    defaultValue={this.state.ms.curvepoints}
                    size="small"
                    type="number"
                    InputProps={{ inputProps:{ min:3 } }}
                    onChange={this.handleMSCurvePointsChange.bind(this)}
                  />

                  {/* checkboxes */}
                  <FormControl component="fieldset" >
                    {/* <FormLabel component="legend">Field</FormLabel> */}
                    <FormGroup>
                      {/* Add noise */}
                      <FormControlLabel
                        control={<Checkbox checked={this.state.ms.noise}
                          onChange={this.handleMSNoiseChange.bind(this)}
                          name="msNoiseCheckbox" />}
                        label="Add noise"
                      />

                      {/* Scale normalize */}
                      <FormControlLabel
                        control={<Checkbox checked={this.state.ms.normalize}
                          onChange={this.handleMSNormalizeChange.bind(this)}
                          name="msNormalizeCheckbox" />}
                        label="Normalize"
                      />
                    </FormGroup>
                    <FormHelperText>Field</FormHelperText>
                  </FormControl>

                  { /* Buttons to recompute M-S and dump crystal partitions to disk */}
                  { /* <ButtonGroup orientation="vertical" >  (available in material-ui v4) */ }
                  { <Button size="small" onClick={this.handleRecomputeMorseSmale.bind(this)}>Recompute</Button> }
                  { this.state.devMode && <Button size="small" onClick={this.handleExportMorseSmale.bind(this)}>
                    Export
                  </Button> }
                  { /* </ButtonGroup> */ }
                </div>
              </AccordionDetails>
            </Accordion>

            { /* Interpolation Model Selection */}
            <Accordion disabled={!this.props.enabled} defaultExpanded={false}
              style={{ paddingLeft:'0px', margin:'1px' }}>
              <AccordionSummary expandIcon={ <ExpandMoreIcon/> }>
                <Typography>Interpolation</Typography>
              </AccordionSummary>
              <AccordionDetails style={{ paddingLeft: '0px',
                paddingRight: '10px', margin: '1px', width: '100%',
                boxSizing: 'border-box' }}>
                <div style={{ display: 'flex', flexDirection: 'column',
                  width: '100%', boxSizing: 'border-box' }}>


                  <FormControl className={classes.formControl}
                    disabled={!this.props.enabled}
                    style={{ width: '100%',
                      boxSizing: 'border-box',
                      paddingRight: '10px' }}>
                    <InputLabel htmlFor='model-field'>Model</InputLabel>
                    <Select ref="interpolationCombo"
                      disabled={!this.props.enabled || !this.props.dataset}
                      value={this.state.interpolationModel || ''}
                      style={{ width:'100%' }}
                      onChange={this.handleInterpolationModelChange.bind(this)}
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
                    label="sigma bandwidth"
                    id="model-sigma"
                    defaultValue={this.state.model.sigma}
                    size="small"
                    onChange={this.handleModelSigmaChange.bind(this)}
                  />

                </div>
              </AccordionDetails>
            </Accordion>

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
                  onChange={this.handlePersistenceSliderChange.bind(this)}
                  onMouseUp={this.handlePersistenceSliderRelease.bind(this)}
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
                    onChange={this.handlePersistenceLevelChange.bind(this)}
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
                    onClick={() => this.handleCrystalToggle(i).bind(this)}>
                    <Checkbox checked={!this.state.crystals[i].isDisabled}
                      tabIndex={-1} disableRipple
                      color="primary" style={{ paddingLeft:0 }}/>
                    <ListItemText primary={'Crystal ' + (i+1)} />
                  </ListItem>
                ))
              }
            </List>
          </div>
        </AccordionDetails>
      </Accordion>
    );
  }
}

// Enforce that Application receives styling.
DecompositionPanel.propTypes = {
  classes: PropTypes.object.isRequired,
};

// Wrap Application in Styling Container.
export default withDSXContext(withStyles({})(DecompositionPanel));
