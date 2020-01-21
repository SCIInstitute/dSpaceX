import EmbeddingWindow from './embeddingWindow';
import MorseSmaleWindow from './morseSmaleWindow';
import React from 'react';
import ResponsiveDrawer from '../components/responsiveDrawer';
import { withDSXContext } from '../dsxContext.js';

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

    this.client = this.props.dsxContext.client;

    this.state = {
      drawerImages: [],
    };

    this.computeNewSamplesUsingShapeoddsModel = this.computeNewSamplesUsingShapeoddsModel.bind(this);
  }

  /**
   * Computes new samples using shapeodds model
   * @param {number} datasetId
   * @param {number} persistenceLevel
   * @param {number} crystalID
   * @param {number} numSamples
   */
  computeNewSamplesUsingShapeoddsModel(datasetId, persistenceLevel, crystalID, numSamples) {
    // eslint-disable-next-line max-len
    console.log('computeNewSamplesUsingShapeoddsModel('+datasetId+','+persistenceLevel+','+crystalID+','+numSamples+')');

    // Ask server to compute the N new images for this crystal and add them to the drawer
    this.client.fetchNImagesForCrystal_Shapeodds(datasetId, persistenceLevel, crystalID, numSamples)
      .then((result) => {
        const thumbnails = result.thumbnails.map((thumbnail, i) => {
          return {
            img: thumbnail,
            id: i,
          };
        });
        this.setState({ drawerImages:thumbnails });
        // eslint-disable-next-line max-len
        console.log('computeNewSamplesUsingShapeoddsModel returned ' + result.thumbnails.length + ' images; msg: ' + result.msg);
      });
  }

  /**
   * Renders EmbeddingMorseSmaleWindow to screen
   * @return {JSX}
   */
  render() {
    let container = {
      display: 'flex',
      flexDirection: 'row',
    };
    return (
      <div style={container}>
        <EmbeddingWindow
          dataset={this.props.dataset}
          decomposition={this.props.decomposition}
          embedding={this.props.embedding}
          selectedDesigns={this.props.selectedDesigns}
          onDesignSelection={this.props.onDesignSelection}
          activeDesigns={this.props.activeDesigns}
          numberOfWindows={this.props.numberOfWindows}/>
        {/* <GraphGLWindow*/}
        {/*  dataset={this.props.dataset}*/}
        {/*  decomposition={this.props.decomposition}*/}
        {/*  selectedDesigns={this.props.selectedDesigns}*/}
        {/*  onDesignSelection={this.props.onDesignSelection}*/}
        {/*  activeDesigns={this.props.activeDesigns}*/}
        {/*  numberOfWindows={this.props.numberOfWindows}/>*/}
        <MorseSmaleWindow
          dataset={this.props.dataset}
          decomposition={this.props.decomposition}
          numberOfWindows={this.props.numberOfWindows}
          onCrystalSelection={this.props.onCrystalSelection}
          evalShapeoddsModelForCrystal={this.computeNewSamplesUsingShapeoddsModel}/>
        <ResponsiveDrawer images={this.state.drawerImages}/>
      </div>
    );
  }
}

export default withDSXContext(EmbeddingMorseSmaleWindow);
