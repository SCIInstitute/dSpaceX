import React from 'react';
import { Drawer } from '@material-ui/core';
import Grid from '@material-ui/core/Grid';
import Paper from '@material-ui/core/Paper';
import withStyles from '@material-ui/core/es/styles/withStyles';

const styles = (theme) => ({
  dragger: {
    height: '100%',
    width: '100%',
    cursor: 'n-resize',
    padding: '5px 5px 5px 5px',
    borderTop: '1px solid #ddd',
    position: 'absolute',
    top: 0,
    bottoms: 0,
    // zIndex: '100',
    backgroundColor: '#f4f7f9',
  },
});

/**
 * This class create a drawer that user can resize
 */
class ResponsiveDrawer extends React.Component {
  constructor(props) {
    super(props);
    this.state = {
      isResizing: false,
      lastDownY: 0,
      newHeight: { height:300 },
    };

    this.handleMouseDown = this.handleMouseDown.bind(this);
    this.handleMouseMove = this.handleMouseMove.bind(this);
    this.handleMouseUp = this.handleMouseUp.bind(this);
  }

  handleMouseDown(e) {
    this.setState({ isResizing:true, lastDownY:e.clientY });
  }

  handleMouseMove(e) {
    if (!this.state.isResizing) {
      return;
    }

    let offsetBottom = document.body.offsetHeight - (e.clientY - document.body.offsetTop);
    let minHeight = 50;
    let maxHeight = 600;
    if (offsetBottom > minHeight && offsetBottom < maxHeight) {
      this.setState({ newHeight:{ height:offsetBottom } });
    }
  }

  handleMouseUp(e) {
    this.setState({ isResizing:false });
  }

  componentDidMount() {
    document.addEventListener('mousemove', (e) => this.handleMouseMove(e));
    document.addEventListener('mouseup', (e) => this.handleMouseUp(e));
  }

  render() {
    const { images } = this.props;
    return (
      images.length > 0 &&
         images.map((thumbnail) =>

             <img alt={'Image:' + thumbnail.id}
               height='45'
               style={{ margin:'5px 5px 5px 5px' }}
               src={'data:image/png;base64, ' + thumbnail.img.rawData}/>
           )
    );
  }
}

export default withStyles(styles)(ResponsiveDrawer);

