import React from 'react';
import ReactResizeDetector from 'react-resize-detector';
import { withDSXContext } from '../dsxContext';

class SizeMonitor extends React.Component {

  constructor(props) {
    super(props);
    this.init = this.init.bind(this);
  }
  
  componentDidMount() {
    this.init();
    //window.addEventListener('resize', this.onResize);
  }
  
  componentWillUnmount() {
    //window.removeEventListener('resize', this.onResize);
  }
  
  init() {
    let canvas = this.refs.embeddingCanvas;
  }
  
  onResize = () => {

    const canvas = this.refs.embeddingCanvas;
    const width = canvas.clientWidth;
    const height = canvas.clientHeight;
    console.log('SizeMonitor.resize: resizing canvas from '+canvas.width+' x '+canvas.height+' to '+width+' x '+height);

  };

  render() {
    const style = {
      height: '100%',
      width: '100%',
    };
    return (
      <ReactResizeDetector handleWidth handleHeight onResize={this.onResize}>
        <canvas ref='embeddingCanvas' style={style}/>
      </ReactResizeDetector>);
  }
}

      // <ReactResizeDetector handleWidth handleHeight onResize={this.onResize}>
      //   <canvas ref='embeddingCanvas' style={style}/>
      // </ReactResizeDetector>);

export default SizeMonitor;
