import React, { Component } from 'react';
import GalleryPanel from '../panels/galleryPanel';
import Grid from '@material-ui/core/Grid';
import Paper from '@material-ui/core/Paper';
import grey from '@material-ui/core/es/colors/grey';
import red from '@material-ui/core/es/colors/red';
import { withDSXContext } from '../dsxContext.js';
import { withStyles } from '@material-ui/core/styles';

const styles = (theme) => ({
  root: {
    overflowY: 'auto',
    overflowX: 'hidden',
  },
});

/**
 *Present all thumbnails and filtering options
 */
class GalleryWindow extends Component {
  /**
   *
   * @param {Object} props
   */
  constructor(props) {
    super(props);
    this.client = this.props.dsxContext.client;
    this.dataHelper = this.props.dsxContext.dataHelper;
    this.state = {
      allThumbnails: [],
      allParameters: [],
      allQois: [],
      thumbnails: [],
      parameters: [],
      qois: [],
      filteredImages: [],
    };

    this.handleImageClick = this.handleImageClick.bind(this);
    this.handleAddFilter = this.handleAddFilter.bind(this);
  }

  componentWillMount() {
    let { datasetId } = this.props.dataset;

    // Get Thumbnails
    this.client.fetchThumbnails(datasetId)
      .then((result) => {
        const thumbnails = result.thumbnails.map((thumbnail, i) => {
          return {
            img: thumbnail,
            id: i,
            isSelected: false,
          };
        });
        this.setState({ thumbnails });
      });

    // Get Parameters
    this.getParameters().then((parameters) => {
      this.setState({ parameters });
    });

    // Get Qois
    this.getQois().then((qois) => {
      this.setState({ qois });
    });
  }

  async getParameters() {
    const { datasetId, parameterNames } = this.props.dataset;
    let parameters = [];
    parameterNames.forEach(async (parameterName) => {
      let parameter =
        await this.client.fetchParameter(datasetId, parameterName);
      parameters.push(parameter);
    });
    return parameters;
  }

  async getQois() {
    const { datasetId, qoiNames } = this.props.dataset;
    let qois = [];
    qoiNames.forEach(async (qoiName) => {
      let qoi =
        await this.client.fetchQoi(datasetId, qoiName);
      qois.push(qoi);
    });
    return qois;
  }

  handleImageClick(id) {
    const thumbnails = [...this.state.thumbnails];
    thumbnails.forEach((thumbnail) => thumbnail.isSelected = false);
    let index = thumbnails.findIndex((thumbnail) => thumbnail.id === id);
    thumbnails[index].isSelected = true;
    this.setState({ thumbnails });
  }

  handleAddFilter(min, max, attributeGroup, attribute) {
    let filteredImages = [];
    if (attributeGroup === 'parameters') {
      let params = this.state.parameters.filter((p) => p.parameterName === attribute)[0].parameter;
      let filteredParams = params.filter((p) => p <= min || p >= max);
      filteredParams.forEach((value) => {
        let index = params.findIndex((v) => v === value);
        filteredImages.push(index);
      });
    } else if (attributeGroup === 'qois') {
      let qois = this.state.qois.filter((q) => q.qoiName === attribute)[0].qoi;
      let filteredQois = qois.filter((q) => q <= min || q >= max);
      filteredQois.forEach((value) => {
        let index = qois.findIndex((v) => v === value);
        filteredImages.push(index);
      });
    }
    this.setState({ filteredImages });
  }

  /**
   *
   * @return {*}
   */
  render() {
    const { classes } = this.props;

    return (
      <Paper className={classes.root}>
        <GalleryPanel parameters={this.state.parameters} qois={this.state.qois} addFilter={this.handleAddFilter}/>
        <Grid container
          justify={'center'}
          spacing={8}
          style={{ margin:'5px 0px 0px 0px' }}>
          {this.state.thumbnails.length > 0
          && this.state.thumbnails.map((thumbnail, i) =>
            !this.state.filteredImages.includes(i) && <Grid key={i} item>
              <Paper
                style={{ backgroundColor:thumbnail.isSelected ? red['700'] : grey['200'] }}>
                <img alt={'Image:' + i} onClick={() => this.handleImageClick(i)} height='75'
                  style={{ margin:'5px 5px 5px 5px' }}
                  src={'data:image/png;base64, ' + thumbnail.img.rawData}/>
              </Paper>
            </Grid>)}
        </Grid>
      </Paper>
    );
  }
}

export default withDSXContext(withStyles(styles)(GalleryWindow));

