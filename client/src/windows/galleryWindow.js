import Button from '@material-ui/core/Button';
import Paper from '@material-ui/core/Paper';
import React, { Component } from 'react';
import { withDSXContext } from '../dsxContext.js';
import { withStyles } from '@material-ui/core/styles';

const styles = (theme) => ({
  root: {
    overflowX: 'auto',
  },
  table: {},
  tableWrapper: {},
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
    this.state = {
      thumbnails: []
    };

    this.handleImageClick = this.handleImageClick.bind(this);
  }

  componentDidMount() {
    const that = this;
    this.client.fetchThumbnails(this.props.dataset.datasetId)
      .then(result => {
        const thumbnails = result.thumbnails.map((thumbnail, i) => {
          return {
            img: thumbnail,
            id: i,
            isSelected: false
          };
        });
        that.setState({ thumbnails });
      });

    // this.client.fetchParameter(this.props.dataset.datasetId)
    //   .then(result => {
    //     console.log(result);
    //   })
  }

  handleImageClick(id) {
    let thumbnails = [...this.state.thumbnails];
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
        <Button variant='contained'
                style={{ margin: '10px 10px 10px 10px', backgroundColor: 'lightGrey' }}>
          Add Filter
        </Button>

        <div>
          {this.state.thumbnails.length > 0 && this.state.thumbnails.map((thumbnail, i) =>
            <img key={i} onClick={() => this.handleImageClick(i)} height='100'
                 style={{
                   margin: '5px 5px 5px 5px',
                   border: thumbnail.isSelected ? 'solid red' : 'solid lightGrey'
                 }}
                 src={'data:image/png;base64, ' + thumbnail.img.rawData}/>)}
        </div>
      </Paper>
    );
  }
}

export default withDSXContext(withStyles(styles)(GalleryWindow));

