package com._4paradigm.rtidb.client.impl;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.List;

import com._4paradigm.rtidb.client.KvIterator;
import com._4paradigm.rtidb.client.TabletException;
import com._4paradigm.rtidb.client.ha.RTIDBClientConfig;
import com._4paradigm.rtidb.client.metrics.TabletMetrics;
import com._4paradigm.rtidb.client.schema.ColumnDesc;
import com._4paradigm.rtidb.client.schema.RowCodec;
import com.google.protobuf.ByteString;

public class DefaultKvIterator implements KvIterator {

    private ByteString bs;
    private int offset;
    private ByteBuffer bb;
    // no copy
    private ByteBuffer slice;
    private int length;
    private long time;
    private int totalSize;
    private List<ColumnDesc> schema;
    private Long network = 0l;
    private Long decode = 0l;
    private int count;
    private RTIDBClientConfig config = null;
    public DefaultKvIterator(ByteString bs) {
        this.bs = bs;
        this.bb = this.bs.asReadOnlyByteBuffer();
        this.offset = 0;
        this.totalSize = this.bs.size();
        next();
    }
    
    public DefaultKvIterator(ByteString bs, RTIDBClientConfig config) {
        this(bs);
        this.config = config;
    }
    
    public int getCount() {
		return count;
	}

	public void setCount(int count) {
		this.count = count;
	}

	public DefaultKvIterator(ByteString bs, List<ColumnDesc> schema) {
        this(bs);
        this.schema = schema;
    }
	
	public DefaultKvIterator(ByteString bs, List<ColumnDesc> schema, RTIDBClientConfig config) {
        this(bs, schema);
        this.config = config;
    }
    
    public DefaultKvIterator(ByteString bs, Long network) {
        this.bs = bs;
        this.bb = this.bs.asReadOnlyByteBuffer();
        this.offset = 0;
        this.totalSize = this.bs.size();
        next();
        this.network = network;
    }
    
    public DefaultKvIterator(ByteString bs, List<ColumnDesc> schema, Long network) {
        this.bs = bs;
        this.bb = this.bs.asReadOnlyByteBuffer();
        this.offset = 0;
        this.totalSize = this.bs.size();
        next();
        this.schema = schema;
        this.network = network;
    }

    public List<ColumnDesc> getSchema() {
		return schema;
	}

	public boolean valid() {
        if (offset <= totalSize) {
            return true;
        }
        if (config!=null && config.isMetricsEnabled()) {
        	    TabletMetrics.getInstance().addScan(decode, network);
        }
        return false;
    }

    public long getKey() {
        return time;
    }

    // no copy
    public ByteBuffer getValue() {
        return slice;
    }
    
    public Object[] getDecodedValue() throws TabletException {
        	Long delta = System.nanoTime();
        	if (schema == null) {
        		throw new TabletException("get decoded value is not supported");
        	}
        	Object[] row = RowCodec.decode(slice, schema);
        	decode += System.nanoTime() - delta;
        	return row;
    }

    public void next() {
    	    Long delta = System.nanoTime();
        if (offset + 4 > totalSize) {
            offset += 4;
            return;
        }
        slice = this.bb.slice().asReadOnlyBuffer().order(ByteOrder.LITTLE_ENDIAN);
        slice.position(offset);
        int size = slice.getInt();
        time = slice.getLong();
        // calc the data size
        length = size - 8;
        if (length < 0) {
            throw new RuntimeException("bad frame data");
        }
        offset += (4 + size);
        slice.limit(offset);
        decode += System.nanoTime() - delta;
    }
}