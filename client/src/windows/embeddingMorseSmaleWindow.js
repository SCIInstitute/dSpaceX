import { Grid, GridList, GridListTile } from '@material-ui/core';
import EmbeddingWindow from './embeddingWindow';
import MorseSmaleWindow from './morseSmaleWindow';
import Paper from '@material-ui/core/Paper';
import React from 'react';
import Typography from '@material-ui/core/Typography';
import { withDSXContext } from '../dsxContext.js';
import { withStyles } from '@material-ui/core/styles';

const styles = (theme) => ({
  root: {
    flexGrow: 1,
  },
  rightPanel: {
    backgroundColor: '#ffffff',
    lineHeight: 0,
  },
  leftPanel: {
    backgroundColor: '#ffffff',
    borderRight: 'solid #A9A9A9',
    lineHeight: 0,
  },
  gridlistRoot: {
    display: 'flex',
    flexWrap: 'wrap',
    justifyContent: 'space-around',
    overflow: 'hidden',
    backgroundColor: theme.palette.background.paper,
    borderTop: 'solid #A9A9A9',
  },
  gridList: {
    flexWrap: 'nowrap',
    width: '100%',
    transform: 'translateZ(0)',
  },
  paper: {
    display: 'flex',
    flexDirection: 'column',
    alignItems: 'center',
    border: 'solid #D3D3D3',
    marginTop: '50%',
  },
});

/**
 * Creates windows that displays the 2D Graph Embedding of the data
 * and the Morse-Smale decomposition
 */
class EmbeddingMorseSmaleWindow extends React.Component {
  /**
   * Creates EmbeddingMorseSmaleWindow object
   * @param {object} props
   */
  constructor(props) {
    super(props);

    this.state = {
      drawerImages: [],
    };

    this.client = this.props.dsxContext.client;

    this.computeNewSamplesUsingShapeoddsModel = this.computeNewSamplesUsingShapeoddsModel.bind(this);
  }

  /**
   * Computes new samples using shapeodds model
   * @param {number} datasetId
   * @param {string} category
   * @param {string} fieldname
   * @param {number} persistenceLevel
   * @param {number} crystalID
   * @param {number} numSamples
   * @param {bool} showOrig
   */
  computeNewSamplesUsingShapeoddsModel(datasetId, category, fieldname, persistenceLevel,
    crystalID, numSamples, showOrig) {
    console.log('computeNewSamplesUsingShapeoddsModel('+datasetId+','+fieldname+','+persistenceLevel+','
                +crystalID+','+numSamples+','+showOrig+')');

    // Ask server to compute the N new images for this crystal and add them to the drawer
    this.client.fetchNImagesForCrystal_Shapeodds(datasetId, category, fieldname, persistenceLevel,
      crystalID, numSamples, showOrig)
      .then((result) => {
        const thumbnails = result.thumbnails.map((thumbnail, i) => {
          return {
            img: thumbnail,
            id: i,
          };
        });
        this.setState({ drawerImages:thumbnails });
        console.log('computeNewSamplesUsingShapeoddsModel returned ' + result.thumbnails.length
          + ' images; msg: ' + result.msg);
      });
  }

  /**
   * Renders EmbeddingMorseSmaleWindow to screen
   * @return {JSX}
   */
  render() {
    const { classes } = this.props;
    const { drawerImages } = this.state;

    return (
      <React.Fragment>
        <Grid container >
          <Grid item xs={6} className={classes.leftPanel}>
            <EmbeddingWindow
              dataset={this.props.dataset}
              decomposition={this.props.decomposition}
              embedding={this.props.embedding}
              selectedDesigns={this.props.selectedDesigns}
              onDesignSelection={this.props.onDesignSelection}
              activeDesigns={this.props.activeDesigns}
              numberOfWindows={this.props.numberOfWindows}/>
          </Grid>
          <Grid item xs={6} className={classes.rightPanel}>
            <MorseSmaleWindow
              dataset={this.props.dataset}
              decomposition={this.props.decomposition}
              numberOfWindows={this.props.numberOfWindows}
              onCrystalSelection={this.props.onCrystalSelection}
              evalShapeoddsModelForCrystal={this.computeNewSamplesUsingShapeoddsModel}/>
          </Grid>
        </Grid>
        <div className={classes.gridlistRoot}>
          {/* {drawerImages.length > 0 && <Typography>Original Designs</Typography>}*/}
          <GridList className={classes.gridList} cols={10}>
            {drawerImages.map((tile) => (
              <GridListTile key={tile.id}>
                <Paper className={classes.paper}>
                  {/* <Typography>{'Design: ' + tile.id}</Typography>*/}
                  <img alt={'Image:' + tile.id}
                    height={tile.img.height}
                    src={'data:image/png;base64, ' + tile.img.rawData}/>
                </Paper>
              </GridListTile>))}
          </GridList>
        </div>
      </React.Fragment>
    );
  }
}

export default withDSXContext(withStyles(styles)(EmbeddingMorseSmaleWindow));
