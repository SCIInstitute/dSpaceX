import EmbeddingWindow from './embeddingWindow';
// import GraphGLWindow from './graphGLWindow';
import MorseSmaleWindow from './morseSmaleWindow';
import React from 'react';
import ResponsiveDrawer from '../components/responsiveDrawer';

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
  }

  camsFunctionGoesHere(persistenceLeve, crystalID) {
    // This will call cams function to get the images we need for the drawer
    // Cams controller function goes here to get the images
    // this.client.fetchSomething.then (result => {
    // set state}
    this.setState({ drawerImages:list_of_images_here });
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
          toCallCamsFunction={this.camsFunctionGoesHere}/>
        <ResponsiveDrawer images={this.state.drawerImages}/>
      </div>
    );
  }
}

export default EmbeddingMorseSmaleWindow;
