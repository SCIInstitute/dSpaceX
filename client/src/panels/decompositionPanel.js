import { Accordion, AccordionDetails, AccordionSummary } from '@material-ui/core';
import { Button, ButtonGroup } from '@material-ui/core';
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
import { saveAs } from 'file-saver';
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

    let defaultField = this.props.dataset.qoiNames ? this.props.dataset.qoiNames[0] : '';
    let defaultModelList = this.props.dataset.qoiNames && props.fieldModels ? props.fieldModels.get(defaultField) : [];
    let defaultModel = this.props.dataset.qoiNames && props.fieldModels ? defaultModelList[0] : '';

    this.state = {
      datasetId: this.props.dataset.datasetId,

      decompositionMode: 'Morse-Smale',

      selection: {
        fieldname: defaultField,
        category: 'qoi',
        modelname: defaultModel,
        modellist: defaultModelList,
      },

      ms: {
        knn: 15,
        sigma: 0.25,
        smooth: 15.0,
        curvepoints: 50,
        depth: 20,
        noise: true,
        normalize: true,
        layout: 'pca2',
      },

      model: {
        sigmaScale: 1,
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
      minPersistence: 0,
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
    const { decompositionMode } = this.state;
    const { fieldname, category, modelname } = this.state.selection;

    // Verify category (geometry and precomputed not yet supported)
    if (!(category == 'parameter' ||
          category == 'qoi')) {
      console.log('decompositionPanel current configuration is invalid:' +
          '\n\tcategory '+category+' not yet supported');
      return false;
    }

    // Ensure a field is selected (and model should at least be 'None'
    if (!fieldname) {
      console.log('decompositionPanel current configuration is invalid:\n\tno field selected');
      return false;
    }

    // Model should at least be 'None'
    if (!modelname) {
      console.log('decompositionPanel current configuration is invalid:\n\tno model selected');
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
        this.clearDecompositionState();
        let datasetId = this.state.datasetId;
        const { fieldname, category } = this.state.selection;
        const { knn, sigma, smooth, noise, depth, curvepoints, normalize } = this.state.ms;
        //console.log('decompositionPanel.fetchDecomposition: fetching decomposition for '+fieldname+' from server...\n');
        await this.client.fetchMorseSmaleDecomposition(datasetId,
          category, fieldname, knn, sigma, smooth, noise, depth, curvepoints, normalize)
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
        prevState.selection.fieldname !== this.state.selection.fieldname) {
      // console.log('decompositionPanel.componentDidUpdate: field changed, fetching new decomposition...');
      this.fetchDecomposition();
    } else if (prevState.persistenceLevel !== this.state.persistenceLevel) {
      // console.log('Persistence level changed from '
      //     + prevState.persistenceLevel +' to '+this.state.persistenceLevel+', updating data model...');
      this.updateDataModel();
    }
    else if (prevState.selection.modelname !== this.state.selection.modelname ||
             prevState.model.sigmaScale !== this.state.model.sigmaScale ||
             prevState.ms.layout !== this.state.ms.layout) {
      this.updatePropsConfig();
    }
    else {
      //console.log('decompositionPanel.componentDidUpdate, but state has not changed.');
    }
  }

  /**
   * Passes config changes up the line.
   * These get used by parallel components such as the EmbeddingMorseSmaleWindow.
   */
  updatePropsConfig() {
    this.props.onDecompositionChange({
      datasetId: this.state.datasetId,
      decompositionMode: this.state.decompositionMode,
      category: this.state.selection.category,
      fieldname: this.state.selection.fieldname,
      persistenceLevel: this.state.persistenceLevel,
      modelname: this.state.selection.modelname,
      sigmaScale: this.state.model.sigmaScale,
      ms: this.state.ms,
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

  handleCurveLayoutChange(event) {
    let layout = event.target.value;
    this.setState((prevState) => ({
      ms: { ...prevState.ms, layout: layout }, 
    }));
  }

  handleRecomputeMorseSmale() {
    this.fetchDecomposition();
  }

  handleExportMorseSmale() {
    this.client.exportMorseSmaleDecomposition().then((response) => {
      let fileName = response['field'].replace(' ', '', 'g') + '_Crystal_Partitions.json';
      let fileToSave = new Blob([JSON.stringify(response, undefined, 2)], {
        type: 'application/json',
        name: fileName,
      });
      saveAs(fileToSave, fileName);
      console.log(response);
    });
  }

  handleModelSigmaScaleChange(event) {
    let sigmaScale = parseInt(event.target.value);
    if (!isNaN(sigmaScale)) {
      this.setState((prevState) => ({
        model: { ...prevState.model, sigmaScale:sigmaScale },
      }));
    }
  }

  /**
   * Handle the decomposition field category changing.
   * @param {object} event
   */
  handleDecompositionCategoryChange(event) {
    let category = event.target.value;
    if (this.state.selection.category != category) {
      let field = '';
      switch (category) {
      case 'parameter':
        field = this.props.dataset.parameterNames[0];
        break;
      case 'qoi':
        field = this.props.dataset.qoiNames[0];
        break;
      default:
        break;
      }
      let modellist = this.props.fieldModels.get(field);
      let model = modellist[0];

      this.setState({
        selection: { fieldname: field,
                     category: category,
                     modelname: model,
                     modellist: modellist, }
      });
    }
  }

  /**
   * Handle the decomposition field changing.
   * @param {object} event
   */
  handleDecompositionFieldChange(event) {
    let field = event.target.value;
    this.setState((prevState) => ({
      selection: { ...prevState.selection,
                   fieldname: field,
                   modelname: this.props.fieldModels.get(field)[0],
                   modellist: this.props.fieldModels.get(field) },
    }));
  }

  /**
   * Handle the interpolation model changing.
   * @param {object} event
   */
  handleInterpolationModelChange(event) {
    let model = event.target.value;
    this.setState((prevState) => ({
      selection: { ...prevState.selection, modelname: model, }
    }));
    // todo: update ms params used for this model and disable their modification
    //       enable modification of m-s params when model is 'None'
  }

  /**
   * Updates state when active persistence level changes.
   * @param {string} level
   */
  async updateDataModel(level) {
    let persistence = this.state.persistenceLevel;
    if (persistence >= this.state.minPersistence && persistence <= this.state.maxPersistence) {
      if (this.state.decompositionMode == 'Morse-Smale') {
        // annoying (and error prone) to have to send all the
        // same parameters to this function as to fetchDecomposition (fixme)
        let datasetId = this.state.datasetId;
        const { fieldname, category } = this.state.selection;
        const { knn, sigma, smooth, noise, depth, curvepoints, normalize } = this.state.ms;
        //console.log('decompositionPanel.updateDataModel: fetching persistence level '+persistence+' of decomposition...\n');
        await this.client.fetchMorseSmalePersistence(datasetId, category, fieldname, persistence,
                                                     knn, sigma, smooth, noise, depth, curvepoints, normalize)
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
    }
    else {
      // console.log('decompositionPanel.updateDataModel failed: \n\tpersistenceLevel (' + persistence +
      //             ') out of range (' + this.state.minPersistence + ', ' + this.state.maxPersistence + ')');
      this.setState({ crystals: [] });
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
    switch (this.state.selection.category) {
      case 'parameter':
        items = this.props.dataset.parameterNames.map((name) => (
          <MenuItem value={name} key={name}>
            <em>{name}</em>
          </MenuItem>
        ));
        break;
      case 'qoi':
        items = this.props.dataset.qoiNames.map((name) => (
          <MenuItem value={name} key={name}>
            <em>{name}</em>
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
      <Accordion disabled={!this.props.enabled || !this.props.dataset}
        defaultExpanded={true} style={{ paddingLeft:'0px', margin:'1px' }}>
        <AccordionSummary expandIcon={ <ExpandMoreIcon/> }>
          <Typography>Decomposition</Typography>
        </AccordionSummary>
        <AccordionDetails style={{ paddingLeft: '1px',
          paddingRight: '0px', margin: '1px', width: '100%',
          boxSizing: 'border-box' }}>
          <div style={{ display: 'flex', flexDirection: 'column',
            width: '100%', boxSizing: 'border-box' }}>

            { /* Field Category (Decomposition) Dropdown */ }
            <FormControl className={classes.formControl}
              disabled={!this.props.enabled || !this.props.dataset}>
              <InputLabel htmlFor='category-input'>Field Category</InputLabel>
              <Select ref="categoryCombo"
                value={this.state.selection.category || ''}
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
                || !this.state.selection.category}>
              <InputLabel htmlFor='field-input'>Field</InputLabel>
              <Select ref="fieldCombo"
                value={this.state.selection.fieldname || ''}
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

            { /* Prediction Model Selection */}
            <Accordion disabled={!this.props.enabled} defaultExpanded={false}
              style={{ paddingLeft:'0px', margin:'1px' }}>
              <AccordionSummary expandIcon={ <ExpandMoreIcon/> }>
                <Typography>Modeling</Typography>
              </AccordionSummary>
              <AccordionDetails style={{ paddingLeft: '0px',
                paddingRight: '0px', margin: '1px', width: '100%',
                boxSizing: 'border-box' }}>
                <div style={{ display: 'flex', flexDirection: 'column',
                  width: '100%', boxSizing: 'border-box' }}>

            {/* Interpolation Model Dropdown */}
            <FormControl className={classes.formControl}
              disabled={!this.props.enabled}
              style={{ width: '100%',
              boxSizing: 'border-box' }}>
              <InputLabel htmlFor='model-field'>Model</InputLabel>
              <Select ref="interpolationCombo"
                value={this.state.selection.modelname}
                style={{ width:'100%' }}
                onChange={this.handleInterpolationModelChange.bind(this)} 
                inputProps={{ name: 'model', id: 'model-field' }}>
                {this.state.selection.modellist.map((modelname) =>
                  <MenuItem value={modelname} key={modelname} >
                    <em>{modelname}</em>
                  </MenuItem>)}
              </Select>
            </FormControl>

                  { /* Interpolation Model [Gaussian] sigma bandwidth parameter */ }
                  <TextField
                    label="Gaussian Sigma Scale"
                    id="model-sigma"
                    defaultValue={this.state.model.sigmaScale}
                    size="small"
                    type="number"
                    InputProps={{ inputProps:{ min:1, max:10 } }}
                    onChange={this.handleModelSigmaScaleChange.bind(this)}
                  />

                </div>
              </AccordionDetails>
            </Accordion>

            {/* Partitioning Algorithm */ }
            <Accordion disabled={!this.props.enabled} defaultExpanded={false}
              style={{ paddingLeft:'0px', margin:'1px' }}>
              <AccordionSummary expandIcon={ <ExpandMoreIcon/> }>
                <Typography>Partitioning</Typography>
              </AccordionSummary>
              <AccordionDetails style={{ paddingLeft: '0px',
                paddingRight: '0px', margin: '1px', width: '100%',
                boxSizing: 'border-box' }}>
                <div style={{ display: 'flex', flexDirection: 'column',
                  width: '100%', boxSizing: 'border-box' }}>

                  <div style={{ height:'30px', display: 'flex', justifyContent: 'center' }}>Computation</div>

                  <FormControl className={classes.formControl}
                    disabled={!this.props.enabled}
                    style={{ width: '100%',
                      boxSizing: 'border-box' }}>
                    <InputLabel htmlFor='mode-field'>Method</InputLabel>
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

                  {/* Smooth */}
                  <TextField
                    label="Regression Bandwidth"
                    id="ms-smooth"
                    defaultValue={this.state.ms.smooth}
                    size="small"
                    onChange={this.handleMSSmoothChange.bind(this)}
                  />

                  {/* Scale normalize checkbox */}
                  <FormControlLabel
                    control={<Checkbox checked={this.state.ms.normalize}
                      onChange={this.handleMSNormalizeChange.bind(this)}
                      name="msNormalizeCheckbox" />}
                    label="Normalize"
                  />

                  {/* Depth */}
                  <TextField
                    label="Max Persistences"
                    id="ms-depth"
                    defaultValue={this.state.ms.depth}
                    size="small"
                    type="number"
                    InputProps={{ inputProps:{ min:-1 } }}
                    onChange={this.handleMSDepthChange.bind(this)}
                  />

                  {/* Sigma */}
                  <TextField
                    label="Curve Smoothing"
                    id="ms-sigma"
                    defaultValue={this.state.ms.sigma}
                    size="small"
                    onChange={this.handleMSSigmaChange.bind(this)}
                  />

                  { /* Buttons to recompute M-S and dump crystal partitions to disk */}
                  <div style={{ height:'10px' }}></div>
                  <ButtonGroup orientation="vertical" >
                    { <Button size="small" onClick={this.handleRecomputeMorseSmale.bind(this)}>Refresh</Button> }
                    { <Button size="small" onClick={this.handleExportMorseSmale.bind(this)}>Save</Button> }
                  </ButtonGroup>

                </div>
              </AccordionDetails>
            </Accordion>

            { /* Histogram of data partitions (each consisting of n subpartitions) */ }
            <div style={{ height:'15px' }}></div>

            <FormControl className={classes.formControl}
                         disabled={!this.props.enabled}
                         style={{ width: '100%',
                         boxSizing: 'border-box' }}>
              <InputLabel htmlFor='layout-field'>Layout</InputLabel>
              <Select ref="curveLayout"
                      disabled={!this.props.enabled || !this.props.dataset}
                      value={this.state.ms.layout}
                      style={{ width:'100%' }}
                      onChange={this.handleCurveLayoutChange.bind(this)}
                      inputProps={{
                      name: 'layout',
                      id: 'layout-field',
                      }}>
                <MenuItem value='iso' key='iso'>
                  <em>Isomap</em>
                </MenuItem>
                <MenuItem value='pca' key='pca'>
                  <em>PCA</em>
                </MenuItem>
                <MenuItem value='pca2' key='pca2'>
                  <em>PCA2</em>
                </MenuItem>
              </Select>
            </FormControl>

            <div style={{ height:'10px' }}></div>
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

      { /* these don't make much sense
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
      */ }
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
