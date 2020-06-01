import { Grid, GridList, GridListTile } from '@material-ui/core';
import EmbeddingWindow from './embeddingWindow';
import MorseSmaleWindow from './morseSmaleWindow';
import Paper from '@material-ui/core/Paper';
import Typography from '@material-ui/core/Typography';
import React from 'react';
import { withDSXContext } from '../dsxContext.js';
import { withStyles } from '@material-ui/core/styles';
import SizeMonitor from './sizeMonitor';
import GalleryWindow from './galleryWindow';

const drawerHeight = 250;
const styles = (theme) => ({
  embeddingMorseSmaleWorkspace: {
    background:'#ffffff',
    display:'flex',
    flexDirection:'column',
    height:'100%',
    justifyContent:'flex-end',
  },
  topPanels: {
    flex:'auto',
    display:'flex',
    flexDirection:'row',
    height:'calc(100vh - '+drawerHeight+'px)',
    justifyContent:'flex-end',
  },
  verticalDivider: {
    background:'#808080',
    width:'5px',
    height:'100%',
    flex:'auto'
  },
  embedding: {
    flex:'auto',
  },
  crystals: {
    flex:'auto',
  },
  drawerDivider: {
    background:'#808080',
    width:'100%',
    height:'5px',
  },
  workspace: {  
    display: 'grid',
    height: drawerHeight,
    gridTemplateColumns: '1fr',
    gridTemplateRows: '1fr',
    gridGap: '0em',
  },
  paper: {
    display: 'flex',
    flexDirection: 'column',
    alignItems: 'top',
    border: 'solid #D3D3D3',
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
        <div className={classes.embeddingMorseSmaleWorkspace} >

          {/* top panel: embedding and crystals */}
          <div className={classes.topPanels} >

            {/* embedding */}
            <div style={{ width:'100%', flex:'auto' }}>
              <EmbeddingWindow className={classes.embedding}
                               dataset={this.props.dataset}
                               decomposition={this.props.decomposition}
                               embedding={this.props.embedding}
                               selectedDesigns={this.props.selectedDesigns}
                               onDesignSelection={this.props.onDesignSelection}
                               activeDesigns={this.props.activeDesigns}
                               numberOfWindows={this.props.numberOfWindows}/>
            </div>

            {/* embedding/crystals vertical divider */}
            <div className={classes.verticalDivider} />

            {/* crystals */}
            <div style={{ width:'100%', flex:'auto' }}>
              <MorseSmaleWindow className={classes.crystals}
                                dataset={this.props.dataset}
                                decomposition={this.props.decomposition}
                                numberOfWindows={this.props.numberOfWindows}
                                onCrystalSelection={this.props.onCrystalSelection}
                                evalShapeoddsModelForCrystal={this.computeNewSamplesUsingShapeoddsModel}/>
            </div>
          </div>

          {/* drawer divider (todo: onMouseDown={this.onResizeDrawer})*/}
          <div className={classes.drawerDivider} />

          {/* scrollable shape cards drawer */}
          <div className={classes.workspace} >
            <Paper style={{ overflow:'hidden auto', border:'1px solid gray' }}>
              <Grid container
                    justify={'flex-start'}
                    spacing={8}
                    style={{ margin:'5px 0px 0px 0px' }}>
                {drawerImages.map((tile, i) =>

                <Grid key={i} item>
                  <Paper className={classes.paper}>
                    <img alt={'Image:' + tile.id} key={i}
                         height='45'
                         style={{ margin:'5px 5px 5px 5px' }}
                         src={'data:image/png;base64, ' + tile.img.rawData}/>
                  </Paper>
                </Grid>)}
              </Grid>
            </Paper>
          </div>
        </div>
    );
  }
}

export default withDSXContext(withStyles(styles) (EmbeddingMorseSmaleWindow));
