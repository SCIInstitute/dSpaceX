import { Grid, GridList, GridListTile } from '@material-ui/core';
import EmbeddingWindow from './embeddingWindow';
import MorseSmaleWindow from './morseSmaleWindow';
import Paper from '@material-ui/core/Paper';
import Typography from '@material-ui/core/Typography';
import React from 'react';
import { withDSXContext } from '../dsxContext.js';
import { withStyles } from '@material-ui/core/styles';
import GalleryWindow from './galleryWindow';

const minTileWidth      = 100;
const minTileHeight     = 80;
const tileWidthMargins  = 58;  // 5+5 image margin + 5+5 paper margin + 20 text + 18 fluff (ugh) 
const tileHeightMargins = 60;  // 5+5 image margin + 5+5 paper margin + 40 text (ugh) 

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
    height:'1px',
  },
  drawer: {  
    display: 'grid',
    gridTemplateColumns: '1fr',
    gridTemplateRows: '1fr',
    gridGap: '0em',
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
    transform: 'translateZ(0)',
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
   * @param {bool} showOrig This doesn't compute new samples, but provides _original_ samples instead.
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
            val: result.fieldvals[i],
            id: i,
          };
        });
        this.setState({ drawerImages:thumbnails });
        console.log('computeNewSamplesUsingShapeoddsModel returned ' + result.thumbnails.length
          + ' images; msg: ' + result.msg);
      });
  }

  /**
   * Return best number of columms for the drawer shape cards.
   */
  numCols() {
    let width = 1650;  // TODO: how to get actual width (this is good for single workspace window at fullscreen 1920x1200)
    if (this.props.screenWidthHack) width /= 2;
    return width / this.getTileWidth(); 
  }

  /**
   * Return best guess at tile width
   */
  getTileWidth() {
    let images = this.state.drawerImages;
    if (images && images.length > 0) {
      let img = images[0].img;
      let tile_width = Math.max(minTileWidth, img.width + tileWidthMargins);
      return tile_width;
    }

    return minTileWidth;
  }

  /**
   * Return best guess at tile height
   */
  getTileHeight() {
    let images = this.state.drawerImages;
    if (images && images.length > 0) {
      let img = images[0].img;
      let tile_height = Math.max(minTileHeight, img.height + tileHeightMargins);
      return tile_height;
    }

    return minTileHeight;
  }

  /**
   * Return scaled image width that fits in this tile (aspect ratio considered)
   */
  scaledImageWidth() {
    let images = this.state.drawerImages;
    if (images && images.length > 0) {
      let img = images[0].img;
      let aspect_ratio = img.width / img.height;
      let img_height = Math.min(img.height, this.getTileHeight() - tileHeightMargins);
      return img_height * aspect_ratio;
    }

    return this.getTileWidth() - tileWidthMargins;
  }

  /**
   * Return scaled image height that fits in this tile 
   */
  scaledImageHeight() {
    let images = this.state.drawerImages;
    if (images && images.length > 0) {
      let img = images[0].img;
      let img_height = Math.min(img.height, this.getTileHeight() - tileHeightMargins);
      return img_height;
    }

    return this.getTileHeight() - tileHeightMargins;
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
          <div className={classes.topPanels} style={{ height:'150px' }} >

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
          <div className={classes.drawer} >      
            <GridList className={classes.gridList} cols={this.numCols()}>
              {drawerImages.map((tile) => (
              <GridListTile key={tile.id} style={{ height:this.getTileHeight() }} >
                <Paper className={classes.paper}>
                  <Typography>{'Design: ' + tile.id}</Typography>
                  <Typography>{tile.val.toFixed(5)}</Typography>

                  <img alt={'Image:' + tile.id} key={tile.id}
                       height={this.scaledImageHeight()}
                       width={this.scaledImageWidth()}
                       style={{ margin:'5px 5px 5px 5px' }}
                       src={'data:image/png;base64, ' + tile.img.rawData}/>
                </Paper>
              </GridListTile>))}
            </GridList>
          </div>
        </div>
    );
  }
}

export default withDSXContext(withStyles(styles)(EmbeddingMorseSmaleWindow));
