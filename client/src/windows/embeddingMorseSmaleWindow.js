import { Grid, GridList, GridListTile } from '@material-ui/core';
import EmbeddingWindow from './embeddingWindow';
import MorseSmaleWindow from './morseSmaleWindow';
import Paper from '@material-ui/core/Paper';
import Typography from '@material-ui/core/Typography';
import React from 'react';
import { withDSXContext } from '../dsxContext.js';
import { withStyles } from '@material-ui/core/styles';
import SizeMonitor from './sizeMonitor';
//import ResponsiveDrawer from '../components/responsiveDrawer';

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
  //add to paper: marginTop: '50%',
  paper: {
    display: 'flex',
    flexDirection: 'column',
    alignItems: 'center',
    border: 'solid #D3D3D3'
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
      <div style={{ background:'#ffffff', display:'flex', flexDirection:'column' }}>

        {/* top panel: embedding and crystals */}
        <div style={{ height:'80%', flex:'auto', display:'flex', flexDirection:'row' }}>

          {/* embedding */}
          <div style={{ width:'100%', flex:'auto' }}>
            <EmbeddingWindow
              dataset={this.props.dataset}
              decomposition={this.props.decomposition}
              embedding={this.props.embedding}
              selectedDesigns={this.props.selectedDesigns}
              onDesignSelection={this.props.onDesignSelection}
              activeDesigns={this.props.activeDesigns}
              numberOfWindows={this.props.numberOfWindows}/>
          </div>

          {/* embedding/crystals vertical divider */}
          <div style={{ background:'#808080', width:'5px', height:'100%' }} />

          {/* crystals */}
          <div style={{ width:'100%', flex:'auto' }}>
            <MorseSmaleWindow
              dataset={this.props.dataset}
              decomposition={this.props.decomposition}
              numberOfWindows={this.props.numberOfWindows}
              onCrystalSelection={this.props.onCrystalSelection}
              evalShapeoddsModelForCrystal={this.computeNewSamplesUsingShapeoddsModel}/>
          </div>
        </div>

        {/* drawer divider (todo: onMouseDown={this.onResizeDrawer})*/}
        <div style={{ background:'#808080', width:'100%', height:'5px' }} />

        {/* shape cards drawer */}
        <Grid container
              justify={'flex-end'}
              spacing={8}
              style={{ height:'20%', margin:'5px 0px 0px 0px' }}>
          {drawerImages.map((tile, i) =>
          <Grid key={i} item>
            <Paper style={{ backgroundColor:'#D3D3D3' }}>
              <img alt={'Image:' + tile.id}
                   height='45'
                   style={{ margin:'5px 5px 5px 5px' }}
                   src={'data:image/png;base64, ' + tile.img.rawData}/>
            </Paper>
          </Grid>)}
        </Grid>
      </div>
    );
  }
}

//handy test which finally helped identify ReactResizeDetector as the culprit for all this crap (well, maybe... yes, I double checked.)
        // <div style={{ width:'100%', height:'100%', background:'#ffffff' }}>
        //   <SizeMonitor />
        // </div>

// responsivedrawer back and works just like before
        // {/* shape cards drawer */}
        // <div style={{ height:'20%', flex:'auto', display:'flex', flexDirection:'row', flexWrap:'nowrap', overflow:'hidden' }}>  {/* justifyContent:'space-around',  */}
        //   <ResponsiveDrawer images={this.state.drawerImages}/>
        // </div>

// going to try to bring back responsivedrawer
        // {/* shape cards drawer */}
        // <div style={{ height:'20%', flex:'auto', display:'flex', flexDirection:'row', flexWrap:'nowrap', overflow:'hidden' }}>  {/* justifyContent:'space-around',  */}
        //   {drawerImages.map((tile) => (
        //   <div key={tile.id} style={{ height:'50px', height:'50px', flex:'auto' }} >
        //     <img alt={'Image:' + tile.id}
        //          src={'data:image/png;base64, ' + tile.img.rawData} />
        //   </div>
        //   ))}
        // </div>

//unfortunately, this still results in incorrect [increasing height] sizes on update. 
      // <div style={{ width:'100%', height:'100%', background:'#ffffff', display:'flex', flexDirection:'column' }}>

      //   {/* top panel: embedding and crystals */}
      //   <div style={{ height:'100%', flex:'auto', display:'flex', flexDirection:'row' }}>

      //     {/* embedding */}
      //     <div style={{ width:'100%', flex:'auto' }}>
      //       <EmbeddingWindow
      //         dataset={this.props.dataset}
      //         decomposition={this.props.decomposition}
      //         embedding={this.props.embedding}
      //         selectedDesigns={this.props.selectedDesigns}
      //         onDesignSelection={this.props.onDesignSelection}
      //         activeDesigns={this.props.activeDesigns}
      //         numberOfWindows={this.props.numberOfWindows}/>
      //     </div>

      //     {/* embedding/crystals vertical divider */}
      //     <div style={{ background:'#808080', width:'5px', height:'100%' }} />

      //     {/* crystals */}
      //     <div style={{ width:'100%', flex:'auto' }}>
      //   {/*
      //       <MorseSmaleWindow
      //         dataset={this.props.dataset}
      //         decomposition={this.props.decomposition}
      //         numberOfWindows={this.props.numberOfWindows}
      //         onCrystalSelection={this.props.onCrystalSelection}
      //         evalShapeoddsModelForCrystal={this.computeNewSamplesUsingShapeoddsModel}/>
      //    */}
      //     </div>
      //   </div>

      //   {/* drawer divider (todo: onMouseDown={this.onResizeDrawer})*/}
      //   <div style={{ background:'#808080', width:'100%', height:'5px' }} />

      //   {/* shape cards drawer */}
      //   <div style={{ height:'20%', flex:'auto', display:'flex', flexDirection:'row', flexWrap:'nowrap' }}>  {/* justifyContent:'space-around', overflow:'hidden' */}
      //     {drawerImages.map((tile) => (
      //     <div key={tile.id} style={{ width:'50px', flex:'auto' }} >
      //       <img alt={'Image:' + tile.id}
      //            src={'data:image/png;base64, ' + tile.img.rawData} />
      //     </div>
      //     ))}
      //   </div>
      // </div>

// this works as last item right before end of ResizablePanels, now trying a gridlist
      //</div>
        // <div style={{ background:'#ffffff', height:'100%', width:'100%' }}>
        //     <EmbeddingWindow
        //       dataset={this.props.dataset}
        //       decomposition={this.props.decomposition}
        //       embedding={this.props.embedding}
        //       selectedDesigns={this.props.selectedDesigns}
        //       onDesignSelection={this.props.onDesignSelection}
        //       activeDesigns={this.props.activeDesigns}
        //       numberOfWindows={this.props.numberOfWindows}/>
        // </div>
      //</ResizablePanels>

export default withDSXContext(withStyles(styles) (EmbeddingMorseSmaleWindow));
