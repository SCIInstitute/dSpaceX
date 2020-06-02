import React, { Component } from 'react';
import Grid from '@material-ui/core/Grid';
import Paper from '@material-ui/core/Paper';
import { withDSXContext } from '../dsxContext.js';

/**
 * Present all the thumbnails as a gallery view. User can filter the gallery view
 * using the global gallery button
 */
class GalleryWindow extends Component {
  /**
   *  Creates gallery window object
   * @param {Object} props from parent component
   */
  constructor(props) {
    super(props);

    this.client = this.props.dsxContext.client;
    this.dataHelper = this.props.dsxContext.dataHelper;

    this.state = {
      thumbnails: [],
    };
  }

  /**
   * Gets data from server when component mounts
   * TODO this needs to be extracted and managed by a data manager
   */
  componentDidMount() {
    let { datasetId } = this.props.dataset;

    // Get Thumbnails
    this.client.fetchThumbnails(datasetId)
      .then((result) => {
        const thumbnails = result.thumbnails.map((thumbnail, i) => {
          return {
            img: thumbnail,
            id: i,
          };
        });
        this.setState({ thumbnails });
      });
  }

  /**
   * Gets the new data when the data set changes
   * @param {object} prevProps
   * @param {object} prevState
   * @param {object} prevContext
   */
  componentDidUpdate(prevProps, prevState, prevContext) {
    let { datasetId } = this.props.dataset;

    // Only loads new data if the dataset has changed
    if (prevProps.dataset.datasetId !== datasetId) {
      // Get Thumbnails
      this.client.fetchThumbnails(datasetId)
        .then((result) => {
          const thumbnails = result.thumbnails.map((thumbnail, i) => {
            return {
              img: thumbnail,
              id: i,
            };
          });
          this.setState({ thumbnails });
        });

      this.setState({ filters:[]});
    }
  }

  /**
   *  Renders the Gallery Window
   * @return {jsx}
   */
  render() {
    const { activeDesigns, selectedDesigns } = this.props;
    return (
      <Paper style={{ overflow:'hidden auto', border:'1px solid gray' }}>
        <Grid container
          justify={'center'}
          spacing={8}
          style={{ margin:'5px 0px 0px 0px' }}>
          {this.state.thumbnails.length > 0
          && this.state.thumbnails.map((thumbnail, i) =>
            activeDesigns.has(i) && <Grid key={i} item>
              <Paper
                style={{ backgroundColor:selectedDesigns.has(thumbnail.id) ? '#FFA500' : '#D3D3D3' }}>
                <img alt={'Image:' + i}
                  onClick={(e) => this.props.onDesignSelection(e, thumbnail.id)}
                  height='75'
                  style={{ margin:'5px 5px 5px 5px' }}
                  src={'data:image/png;base64, ' + thumbnail.img.rawData}/>
              </Paper>
            </Grid>)}
        </Grid>
      </Paper>
    );
  }
}

export default withDSXContext(GalleryWindow);

