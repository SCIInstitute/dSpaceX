import EmbeddingWindow from './embeddingWindow';
import {Grid, GridList} from '@material-ui/core';
import MorseSmaleWindow from './morseSmaleWindow';
import Paper from '@material-ui/core/Paper';
import React from 'react';
import { withDSXContext } from '../dsxContext.js';
import { withStyles } from '@material-ui/core/styles';
import GridListTile from "@material-ui/core/GridListTile";
import Typography from "@material-ui/core/Typography";


const styles = (theme) => ({
  root: {
    flexGrow: 1,
    height: '100%',
  },
  panels: {
    backGround: '#ffffff',
    border: '1px solid #A9A9A9',
  },
  gridlistRoot: {
    display: 'flex',
    flexWrap: 'wrap',
    justifyContent: 'space-around',
    overflow: 'hidden',
    backgroundColor: theme.palette.background.paper,
  },
  gridList: {
    flexWrap: 'nowrap',
    // Promote the list into his own layer on Chrome. This cost memory but helps keeping high FPS.
    transform: 'translateZ(0)',
  },
  paper: {
    display: 'flex',
    flexDirection: 'column',
    alignItems: 'center',
  },
  title: {
    color: theme.palette.primary.light,
  },
  titleBar: {
    background:
        'linear-gradient(to top, rgba(0,0,0,0.7) 0%, rgba(0,0,0,0.3) 70%, rgba(0,0,0,0) 100%)',
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
        <Grid container style={{ height:'80%' }}>
          <Grid item xs={6}>
            {/* <div>*/}
            <EmbeddingWindow
              dataset={this.props.dataset}
              decomposition={this.props.decomposition}
              embedding={this.props.embedding}
              selectedDesigns={this.props.selectedDesigns}
              onDesignSelection={this.props.onDesignSelection}
              activeDesigns={this.props.activeDesigns}
              numberOfWindows={this.props.numberOfWindows}/>
            {/* </div>*/}
          </Grid>
          <Grid item xs={6}>
            {/* <div>*/}
            <MorseSmaleWindow
              dataset={this.props.dataset}
              decomposition={this.props.decomposition}
              numberOfWindows={this.props.numberOfWindows}
              onCrystalSelection={this.props.onCrystalSelection}
              evalShapeoddsModelForCrystal={this.computeNewSamplesUsingShapeoddsModel}/>
            {/* </div>*/}
          </Grid>
          {/* <Grid item xs={12} style={{ height:'100px' }}>*/}
          {/*  {drawerImages.length > 0 && drawerImages.map((thumbnail) =>*/}
          {/*    <img alt={'Image:' + thumbnail.id}*/}
          {/*      height='45'*/}
          {/*      width='45'*/}
          {/*      style={{ margin:'5px 5px 5px 5px' }}*/}
          {/*      src={'data:image/png;base64, ' + thumbnail.img.rawData}/>)}*/}
          {/* </Grid>*/}
        </Grid>
        <div className={classes.gridlistRoot}>
          <GridList className={classes.gridList} cols={12}>
            {drawerImages.map((tile) => (
            <GridListTile key={tile.id}>
              <Paper className={classes.paper}>
                <Typography>{'Design: ' + tile.id}</Typography>
                <img alt={'Image:' + tile.id}
                     height={tile.img.height}
                     width={tile.img.width}
                     src={'data:image/png;base64, ' + tile.img.rawData}/>
              </Paper>
            </GridListTile>))}
          </GridList>
          {/*{drawerImages.map((thumbnail) =>*/}
          {/*  <img alt={'Image:' + thumbnail.id}*/}
          {/*    height={thumbnail.img.height}*/}
          {/*    width={thumbnail.img.width}*/}
          {/*    style={{ margin:'5px 5px 5px 5px' }}*/}
          {/*    src={'data:image/png;base64, ' + thumbnail.img.rawData}/>)}*/}
        </div>
      </React.Fragment>
    );
  }
}
//   render() {
//     const { classes } = this.props;
//     const { drawerImages } = this.state;
//
//     return (
//       <div className={classes.main}>
//         <div className={classes.topContainer}>
//           <div style={{ width:'50%' }}>
//             <EmbeddingWindow
//               dataset={this.props.dataset}
//               decomposition={this.props.decomposition}
//               embedding={this.props.embedding}
//               selectedDesigns={this.props.selectedDesigns}
//               onDesignSelection={this.props.onDesignSelection}
//               activeDesigns={this.props.activeDesigns}
//               numberOfWindows={this.props.numberOfWindows}/>
//           </div>
//           <div className={classes.verticalDivider}/>
//           <div style={{ width:'50%' }}>
//             <MorseSmaleWindow
//               dataset={this.props.dataset}
//               decomposition={this.props.decomposition}
//               numberOfWindows={this.props.numberOfWindows}
//               onCrystalSelection={this.props.onCrystalSelection}
//               evalShapeoddsModelForCrystal={this.computeNewSamplesUsingShapeoddsModel}/>
//           </div>
//         </div>
//         <div className={classes.horizontalDivider}/>
//         <div className={classes.bottomPanel}>
//           {drawerImages.length > 0 && drawerImages.map((thumbnail) =>
//             <span>thumbnail.id</span>)}
//         </div>
//       </div>
//     );
//   }
// }

export default withDSXContext(withStyles(styles)(EmbeddingMorseSmaleWindow));

// return (
//   <ResizablePanels
//     bkcolor='#ffffff'
//     displayDirection='column'
//     width='100%'
//     height='100%'
//     panelsSize={[72, 25]}
//     sizeUnitMeasure='%'
//     resizerColor='#808080'
//     resizerSize='1px'>
//     <div style={{ background:'#ffffff', height:'100%', width:'100%' }}>
//       <ResizablePanels
//         bkcolor='#ffffff'
//         displayDirection='row'
//         width='100%'
//         height='100%'
//         panelsSize={[50, 50]}
//         sizeUnitMeasure='%'
//         resizerColor='#808080'
//         resizerSize='1px'>
//         <EmbeddingWindow
//           dataset={this.props.dataset}
//           decomposition={this.props.decomposition}
//           embedding={this.props.embedding}
//           selectedDesigns={this.props.selectedDesigns}
//           onDesignSelection={this.props.onDesignSelection}
//           activeDesigns={this.props.activeDesigns}
//           numberOfWindows={this.props.numberOfWindows}/>
//         <MorseSmaleWindow
//           dataset={this.props.dataset}
//           decomposition={this.props.decomposition}
//           numberOfWindows={this.props.numberOfWindows}
//           onCrystalSelection={this.props.onCrystalSelection}
//           evalShapeoddsModelForCrystal={this.computeNewSamplesUsingShapeoddsModel}/>
//       </ResizablePanels>
//     </div>
//     <div>
//       <ResponsiveDrawer images={this.state.drawerImages}/>
//     </div>
//   </ResizablePanels>
// );
