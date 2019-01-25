import grey from '@material-ui/core/es/colors/grey';
import Grid from '@material-ui/core/Grid';
import Paper from '@material-ui/core/Paper';
import React, { Component } from 'react';
import red from '@material-ui/core/es/colors/red';
import { withDSXContext } from '../dsxContext.js';
import { withStyles } from '@material-ui/core/styles';
import GalleryPanel from '../panels/galleryPanel';

const styles = (theme) => ({
  root: {
    overflowY: 'auto',
    overflowX: 'hidden',
  }
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
      thumbnails: [],
      parameters: [],
      qois: {}
    };

    this.handleImageClick = this.handleImageClick.bind(this);
  }

  componentDidMount() {
    let { datasetId } = this.props.dataset;

    // Get Thumbnails
    this.client.fetchThumbnails(datasetId)
      .then(result => {
        const thumbnails = result.thumbnails.map((thumbnail, i) => {
          return {
            img: thumbnail,
            id: i,
            isSelected: false
          };
        });
        this.setState({ thumbnails });
      });

    // Get Parameters
    this.getParameters().then(parameters => {
      this.setState({ parameters });
    });
  }

  async getParameters() {
    const { datasetId, parameterNames } = this.props.dataset;
    let parameters = [];
    for (let i = 0; i < parameterNames.length; ++i) {
      let parameter = await this.client.fetchParameter(datasetId, parameterNames[i]);
      parameters.push(parameter);
    }
    return parameters;
  }

  handleImageClick(id) {
    const thumbnails = [...this.state.thumbnails];
    thumbnails.forEach(thumbnail => thumbnail.isSelected = false);
    let index = thumbnails.findIndex(thumbnail => thumbnail.id === id);
    thumbnails[index].isSelected = true;
    this.setState({ thumbnails });
  }

  /**
   *
   * @return {*}
   */
  render() {
    const { classes } = this.props;

    return (
      <Paper className={classes.root}>
        <GalleryPanel/>
        <Grid container justify={'center'} spacing={8} style={{ margin: '5px 0px 0px 0px' }}>
          {this.state.thumbnails.length > 0 && this.state.thumbnails.map((thumbnail, i) =>
            <Grid key={i} item>
              <Paper style={{ backgroundColor: thumbnail.isSelected ? red['700'] : grey['200'] }}>
                <img alt={'Image:' + i} onClick={() => this.handleImageClick(i)} height='75'
                     style={{
                       margin: '5px 5px 5px 5px'
                     }}
                     src={'data:image/png;base64, ' + thumbnail.img.rawData}/>
              </Paper>
            </Grid>)}
        </Grid>
      </Paper>
    );
  }
}

export default withDSXContext(withStyles(styles)(GalleryWindow));

